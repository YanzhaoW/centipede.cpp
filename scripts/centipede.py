import math
import os
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import capnp
import numpy as np
import pandas as pd
from odrpack import odr_fit
from odrpack.result import F64Array
from tqdm import tqdm


@dataclass
class Buffer:
    x_data: np.ndarray = field(
        default_factory=lambda: np.array([], dtype=np.float64)
    )
    x_err: np.ndarray = field(
        default_factory=lambda: np.array([], dtype=np.float64)
    )
    y_data: np.ndarray = field(
        default_factory=lambda: np.array([], dtype=np.float64)
    )
    y_err: np.ndarray = field(
        default_factory=lambda: np.array([], dtype=np.float64)
    )
    loss: float = 0.0
    beta: np.ndarray = field(
        default_factory=lambda: np.array([0.0, 0.0], dtype=np.float64)
    )
    global_point_map: dict[tuple[int, int], int] = field(default_factory=dict)
    local_point_map: dict[tuple[int, int], int] = field(default_factory=dict)

    n_locals = 0
    event_idx: int = 0


class Centipede:
    def __init__(
        self, schema_path: str, n_globals: int, v_lambda: float, v_eta: float
    ):
        self._n_globals = n_globals
        self._entry_capnp = self._setup_schema_path(schema_path)
        self._data_filename: str = ""
        self._global_init_pars = {
            par_id: 0.0 for par_id in range(0, n_globals)
        }
        self._lambda = v_lambda
        self._eta = v_eta
        self._buffer = Buffer()
        self._losses = pd.DataFrame()

    @staticmethod
    def linear_function(coefficients: F64Array, pars: F64Array) -> F64Array:
        return pars @ coefficients

    def _setup_schema_path(self, schema_path: str):
        os.environ["PWD"] = str(Path.cwd())
        capnp.remove_import_hook()  # pyright: ignore[reportAttributeAccessIssue]
        # entry_capnp = capnp.load(schema_path, imports=[str(Path(sys.prefix) / "include")])
        return capnp.load(  # pyright: ignore[reportAttributeAccessIssue]
            schema_path,
            imports=[os.path.dirname(p) for p in capnp.__path__],
        )

    def read(self, data_path):
        with open(data_path, "rb") as file:
            self._entry_reader = self._entry_capnp.Entry.read_multiple_packed(
                file
            )

    def analyze(self, event_num: int = -1):
        with open(self._data_filename, "rb") as file:
            reader = self._entry_capnp.Entry.read_multiple_packed(file)
            print("Reading entries ...")
            losses = []
            for event_idx, entry in enumerate(tqdm(reader)):
                self._buffer.event_idx = event_idx
                if (event_num >= 0) and event_idx > event_num:
                    break

                n_locals = self._scan_entry(entrypoints=entry.entrypoint)
                self._buffer.beta = self._local_fit(
                    entrypoints=entry.entrypoint, n_locals=n_locals
                )
                self._buffer.loss = self._calculate_loss(
                    entrypoints=entry.entrypoint, local_inits=self._buffer.beta
                )
                losses.append(self._buffer.loss)
                # self._analyze_entry(entry.entrypoint)
            self._losses = pd.DataFrame({"losses": losses})
            print(f"{len(losses)} events analyzed")

    def _local_fit(self, entrypoints: Any, n_locals: int) -> np.ndarray:
        n_points = len(entrypoints)
        self._buffer.x_data = np.zeros(shape=(n_locals, n_points), dtype=float)
        self._buffer.x_err = np.zeros(shape=(n_locals, n_points), dtype=float)
        self._buffer.y_data = np.zeros(shape=n_points, dtype=float)
        self._buffer.y_err = np.zeros(shape=n_points, dtype=float)
        for point_idx, entrypoint in enumerate(entrypoints):
            self._buffer.y_data[point_idx] = sum(
                [
                    self._global_init_pars[par.id] * par.value
                    for par in entrypoint.globalDerivs
                ]
            )
            for local_deriv in entrypoint.localDerivs:
                self._buffer.x_data[local_deriv.id][
                    point_idx
                ] = local_deriv.value
                self._buffer.x_err[local_deriv.id][
                    point_idx
                ] = local_deriv.error

        input_data = np.vstack([self._buffer.y_data, self._buffer.x_data])
        input_errs = np.vstack([self._buffer.y_err, self._buffer.x_data])
        fixed_vars = input_errs <= 0
        input_errs[fixed_vars] = 1
        fixed_pars = np.zeros(n_locals + 1, dtype=bool)
        fixed_pars[0] = True

        result = odr_fit(
            f=Centipede.linear_function,
            xdata=input_data,
            ydata=np.zeros_like(self._buffer.y_data),
            beta0=np.ones(n_locals + 1),
            weight_x=1 / input_errs**2,
            fix_x=fixed_vars,
            fix_beta=fixed_pars,
            task="implicit-ODR",
        )
        return result.beta[1:]

    def _analyze_entry(self, entrypoints: Any, n_locals: int):
        mat_c_1 = self._calculate_mat_c_1(entrypoints)
        mat_c_2 = self._calculate_mat_c_2(entrypoints)
        for entrypoint in entrypoints:
            print(f"local derivs: {entrypoint.localDerivs}")
            print(f"global derivs: {entrypoint.globalDerivs}")
        pass

    def _calculate_mat_c_1(self, entrypoints: Any):
        mat_c_1 = np.zeros(
            shape=(self._n_globals, self._n_globals), dtype=float
        )
        np.fill_diagonal(mat_c_1, self._lambda)
        for point_idx, entrypoint in enumerate(entrypoints):
            globals = entrypoint.globalDerivs
            for par_left, par_right in zip(globals, globals):
                mat_c_1[par_left.id, par_right.id] += (
                    par_left.value * par_right.value
                )

    def _calculate_mat_c_2(self, entrypoints: Any):
        mat_c_2 = np.zeros(
            shape=(
                self._buffer.n_locals
                + len(self._buffer.global_point_map)
                + self._buffer.n_locals * len(entrypoints)
            )
        )
        mat_c_2_11 = np.zeros(
            shape=(self._buffer.n_locals, self._buffer.n_locals)
        )
        np.fill_diagonal(mat_c_2_11, self._lambda)
        mat_c_2_22 = np.zeros(
            shape=(
                len(self._buffer.global_point_map),
                len(self._buffer.global_point_map),
            )
        )
        mat_c_2_33 = np.zeros(
            shape=(
                len(self._buffer.local_point_map),
                len(self._buffer.local_point_map),
            )
        )
        mat_c_2_12 = np.zeros(
            shape=(self._buffer.n_locals, len(self._buffer.global_point_map))
        )
        mat_c_2_13 = np.zeros(
            shape=(self._buffer.n_locals, len(self._buffer.local_point_map))
        )
        mat_c_2_23 = np.zeros(
            shape=(
                len(self._buffer.global_point_map),
                len(self._buffer.local_point_map),
            )
        )

        for point_idx, entrypoint in enumerate(entrypoints):
            locals = entrypoint.localDerivs
            globals = entrypoint.globalDerivs
            local_sum = sum(
                [self._buffer.beta[par.id] * par.value for par in locals]
            )

            # zip between locals and locals
            for local_left, local_right in zip(locals, locals):
                mat_c_2_11[local_left.id, local_right.id] += (
                    self._eta * local_left.id * local_right.id
                )

                if (
                    local_left.id,
                    point_idx,
                ) in self._buffer.local_point_map and (
                    local_right.id,
                    point_idx,
                ) in self._buffer.local_point_map:
                    map_idx_left = self._buffer.local_point_map[
                        local_left.id, point_idx
                    ]
                    map_idx_right = self._buffer.local_point_map[
                        local_right.id, point_idx
                    ]
                    if local_left.id == local_right.id:
                        # TODO: error may be zero
                        mat_c_2_33[map_idx_left, map_idx_right] += (
                            self._lambda + 1 / local_left.error**2
                        )
                    mat_c_2_33[map_idx_left, map_idx_right] += (
                        self._eta
                        * self._buffer.beta[local_left.id]
                        * self._buffer.beta[local_right.id]
                    )

                if (
                    local_right.id,
                    point_idx,
                ) in self._buffer.local_point_map:
                    map_idx = self._buffer.local_point_map[
                        local_right.id, point_idx
                    ]
                    if local_left.id == local_right.id:
                        mat_c_2_12[local_left.id, map_idx] += (
                            self._eta * local_sum
                        )
                    mat_c_2_12[local_left.id, map_idx] += (
                        self._eta
                        * self._buffer.beta[local_right.id]
                        * local_left.value
                    )

            # zip between locals and globals
            for global_left, local_right in zip(globals, locals):
                if (
                    global_left.id,
                    point_idx,
                ) in self._buffer.global_point_map:
                    map_idx = self._buffer.global_point_map[
                        global_left.id, point_idx
                    ]
                    mat_c_2_12[local_right.id, map_idx] += (
                        self._eta
                        * local_right.value
                        * self._global_init_pars[global_left.id]
                    )

                if (
                    global_left.id,
                    point_idx,
                ) in self._buffer.global_point_map and (
                    local_right.id,
                    point_idx,
                ) in self._buffer.local_point_map:
                    map_idx_global = self._buffer.global_point_map[
                        global_left.id, point_idx
                    ]
                    map_idx_local = self._buffer.local_point_map[
                        local_right.id, point_idx
                    ]
                    mat_c_2_23[map_idx_global, map_idx_local] += (
                        self._eta
                        * self._global_init_pars[global_left.id]
                        * self._buffer.beta[local_right.id]
                    )

            # zip between globals and globals
            for global_left, global_right in zip(globals, globals):

                if (
                    global_left.id,
                    point_idx,
                ) in self._buffer.global_point_map and (
                    global_right.id,
                    point_idx,
                ) in self._buffer.global_point_map:

                    map_idx_left = self._buffer.global_point_map[
                        global_left.id, point_idx
                    ]
                    map_idx_right = self._buffer.global_point_map[
                        global_right.id, point_idx
                    ]
                    if global_left.id == global_right.id:
                        # TODO: error may be zero
                        mat_c_2_22[map_idx_left, map_idx_right] += (
                            self._lambda + 1 / global_left.error**2
                        )

                    mat_c_2_22[map_idx_left, map_idx_right] += (
                        self._eta
                        * self._global_init_pars[global_left.id]
                        * self._global_init_pars[global_right.id]
                    )
        mat_c_2 = np.block(
            [
                [mat_c_2_11, mat_c_2_12, mat_c_2_13],
                [mat_c_2_12.T, mat_c_2_22, mat_c_2_23],
                [mat_c_2_13.T, mat_c_2_23.T, mat_c_2_33],
            ]
        )
        return mat_c_2

    def _scan_entry(self, entrypoints: Any):
        n_local_max_id = 0
        self._buffer.global_point_map.clear()
        self._buffer.local_point_map.clear()
        self._buffer.n_locals = 0

        global_map_idx = 0
        local_map_idx = 0
        for point_idx, entrypoint in enumerate(entrypoints):
            self._buffer.global_point_map.update(
                {
                    (par.id, point_idx): global_map_idx
                    for par in entrypoint.globalDerivs
                    if par.error > 0
                }
            )
            self._buffer.local_point_map.update(
                {
                    (par.id, point_idx): global_map_idx
                    for par in entrypoint.localDerivs
                    if par.error > 0
                }
            )
            global_map_idx += 1
            local_map_idx += 1

            n_local_max_id = max(
                n_local_max_id, max([par.id for par in entrypoint.localDerivs])
            )
        self._buffer.n_locals = n_local_max_id + 1
        return n_local_max_id + 1

    def _calculate_loss(self, entrypoints: Any, local_inits: np.ndarray):
        loss = 0.0
        for entrypoint in entrypoints:
            point_diff = sum(
                [
                    self._global_init_pars[par.id] * par.value
                    for par in entrypoint.globalDerivs
                ]
            ) + sum(
                [
                    local_inits[par.id] * par.value
                    for par in entrypoint.localDerivs
                ]
            )
            loss += point_diff**2
        return loss

    @property
    def buffer(self):
        """The buffer property."""
        return self._buffer

    @buffer.setter
    def buffer(self, value):
        self._buffer = value

    @property
    def entry_reader(self):
        return self._entry_reader

    @property
    def data_filename(self):
        """The data_filename property."""
        return self._data_filename

    @data_filename.setter
    def data_filename(self, value):
        self._data_filename = value

    @property
    def global_init_pars(self):
        """The global_init_pars property."""
        return self._global_init_pars

    @global_init_pars.setter
    def global_init_pars(self, value):
        self._global_init_pars = value

    @property
    def losses(self):
        """The losses property."""
        return self._losses

    @losses.setter
    def losses(self, value):
        self._losses = value
