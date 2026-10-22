import logging
import math
import os
import sys
from dataclasses import dataclass, field
from itertools import islice
from pathlib import Path
from typing import Any

import capnp
import numpy as np
import pandas as pd
from odrpack import odr_fit
from odrpack.result import F64Array
from tqdm import tqdm
from tqdm.contrib.logging import logging_redirect_tqdm


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
    mat_c_1: np.ndarray = field(
        default_factory=lambda: np.array([], dtype=np.float64)
    )
    mat_gamma: np.ndarray = field(
        default_factory=lambda: np.array([], dtype=np.float64)
    )
    mat_g: np.ndarray = field(
        default_factory=lambda: np.array([], dtype=np.float64)
    )
    entrypoints: Any = None
    mat_gamma_l: np.ndarray = field(
        default_factory=lambda: np.array([], dtype=np.float64)
    )

    global_sum: float = 0.0
    n_locals = 0
    event_idx: int = 0


class Centipede:
    def __init__(
        self,
        schema_path: str,
        n_globals: int,
        v_lambda: float,
        v_eta: float,
        fixed_pars: list[int] = [],
    ):
        self._logger = logging.getLogger(self.__class__.__name__)
        self._logger.setLevel(logging.INFO)
        self._logger.propagate = False
        if not self._logger.handlers:
            handler = logging.StreamHandler(sys.stdout)
            handler.setFormatter(
                logging.Formatter("%(levelname)s:%(name)s:%(message)s")
            )
            self._logger.addHandler(handler)

        self._n_globals = n_globals
        self.fixed_globals = fixed_pars

        self._entry_capnp = self._setup_schema_path(schema_path)
        self._data_filename: str = ""
        self._global_init_pars: dict[int, float] = {
            par_id: 0.0 for par_id in range(0, n_globals)
        }
        self._lambda = v_lambda
        self._eta = v_eta
        self._buffer = Buffer()
        self._losses = pd.DataFrame()
        self._rhs = np.zeros(shape=(self._n_fit_globals, 1))
        self._par_results: dict[int, float] = {}
        self._factor_mat = np.zeros(
            shape=(self._n_fit_globals, self._n_fit_globals)
        )

    def set_log_level(self, level):
        self._logger.setLevel(level)

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

    @staticmethod
    def cholesky_solve(L, rhs):
        temp = np.linalg.solve(L, rhs)
        return np.linalg.solve(L.conj().T, temp)

    def _log_matrix_dimension(self, name: str, mat: np.ndarray):
        self._logger.debug(f"Creating {name} with dimension: {mat.shape}")

    def analyze(self, event_num: int = -1):
        with open(self._data_filename, "rb") as file:
            reader = self._entry_capnp.Entry.read_multiple_packed(file)
            self._logger.info("Reading entries ...")
            losses = []
            with logging_redirect_tqdm(
                loggers=[self._logger], tqdm_class=tqdm
            ):
                n_loops = event_num if event_num >= 0 else None
                for event_idx, entry in enumerate(
                    tqdm(islice(reader, n_loops), total=n_loops)
                ):
                    self._buffer.event_idx = event_idx

                    self._buffer.entrypoints = entry.entrypoint
                    self._buffer.n_locals = self._scan_entry(
                        entrypoints=entry.entrypoint, event_idx=event_idx
                    )
                    self._buffer.beta = self._local_fit(
                        entrypoints=entry.entrypoint,
                        n_locals=self._buffer.n_locals,
                    )
                    self._logger.debug(f"Local fits: {self._buffer.beta}")
                    self._buffer.loss = self._calculate_loss(
                        entrypoints=entry.entrypoint,
                        local_inits=self._buffer.beta,
                    )
                    losses.append(self._buffer.loss)
                    try:
                        self._analyze_entry(entry.entrypoint)
                    except np.linalg.LinAlgError as err:
                        eigen_values = np.linalg.eigvals(
                            self._buffer.mat_gamma
                        )
                        with np.printoptions(precision=2, floatmode="fixed"):
                            self._logger.error(
                                f"Cholesky failed: {err}. Eigen values: {eigen_values}"
                            )

            self._losses = pd.DataFrame({"losses": losses})
            self._logger.info(f"{len(losses)} events analyzed")

    def solve(self):
        try:
            mat_factor_l = np.linalg.cholesky(self._factor_mat)
            par_results = Centipede.cholesky_solve(mat_factor_l, self._rhs)
            self._set_par_results(par_results)
        except np.linalg.LinAlgError as err:
            eigen_values = np.linalg.eigvals(self._factor_mat)
            with np.printoptions(precision=2, floatmode="fixed"):
                self._logger.error(
                    f"Cholesky failed: {err}. Eigen values: {eigen_values}"
                )

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

    def _analyze_entry(self, entrypoints: Any):
        self._buffer.mat_c_1 = self._calculate_mat_c_1(entrypoints)
        self._buffer.mat_gamma = self._calculate_mat_gamma(entrypoints)
        self._buffer.mat_g = self._calculate_mat_g(entrypoints)

        self._buffer.mat_gamma_l = np.linalg.cholesky(self._buffer.mat_gamma)
        self._factor_mat += (
            self._buffer.mat_c_1
            - self._buffer.mat_g
            @ Centipede.cholesky_solve(
                self._buffer.mat_gamma_l, self._buffer.mat_g.T
            )
        )
        self._rhs += self._calculate_rhs(entrypoints)

        pass

    def _set_par_results(self, par_results: np.ndarray):
        par_results = par_results.ravel()
        for id in range(0, self._n_globals):
            if (mat_id := self._global_id_convert_map.get(id)) is not None:
                self._par_results[id] = par_results[mat_id]
            else:
                self._par_results[id] = self._global_init_pars[id]

    def _calculate_rhs(self, entrypoints: Any):
        mat_g1 = np.array(
            [
                self._lambda * self._global_init_pars[idx]
                for idx in range(0, self._n_globals)
                if idx in self._global_id_convert_map
            ]
        ).reshape(-1, 1)
        self._log_matrix_dimension("mat_g1", mat_g1)

        mat_d1 = np.array(
            [
                self._lambda * self._buffer.beta[idx]
                for idx in range(0, self._buffer.n_locals)
            ]
        ).reshape(-1, 1)
        self._log_matrix_dimension("mat_d1", mat_d1)

        mat_d2 = np.zeros(shape=(len(self._buffer.global_point_map), 1))
        self._log_matrix_dimension("mat_d2", mat_d2)

        mat_d3 = np.zeros(shape=(len(self._buffer.local_point_map), 1))
        self._log_matrix_dimension("mat_d3", mat_d3)

        for point_idx, entrypoint in enumerate(entrypoints):
            entry_sum = sum(
                [
                    global_par.value * self._global_init_pars[global_par.id]
                    + local_par.value * self._buffer.beta[local_par.id]
                    for global_par, local_par in zip(
                        entrypoint.globalDerivs, entrypoint.localDerivs
                    )
                ]
            )

            globals = entrypoint.globalDerivs
            locals = entrypoint.localDerivs

            for par in globals:
                if (
                    global_mat_id := self._global_id_convert_map.get(par.id)
                ) is not None:
                    mat_g1[global_mat_id, 0] += (
                        self._eta * entry_sum * par.value
                    )

                if (
                    map_idx := self._buffer.global_point_map.get(
                        (par.id, point_idx)
                    )
                ) is not None:
                    mat_d2[map_idx] += (
                        self._eta * entry_sum * self._global_init_pars[par.id]
                    )

            for par in locals:
                mat_d1[par.id, 0] += self._eta * entry_sum * par.value
                if (
                    map_idx := self._buffer.local_point_map.get(
                        (par.id, point_idx)
                    )
                ) is not None:
                    mat_d3[map_idx] += (
                        self._eta * entry_sum * self._buffer.beta[par.id]
                    )

        mat_g2 = np.block([[mat_d1], [mat_d2], [mat_d3]])
        return mat_g1 - self._buffer.mat_g @ Centipede.cholesky_solve(
            self._buffer.mat_gamma_l, mat_g2
        )

    def _calculate_mat_g(self, entrypoints: Any):
        mat_g_1 = np.zeros(
            shape=(self._n_fit_globals, self._buffer.n_locals), dtype=float
        )
        self._logger.debug(f"Creating mat_g_1 with dimension: {mat_g_1.shape}")
        mat_g_2 = np.zeros(
            shape=(self._n_fit_globals, len(self._buffer.global_point_map)),
            dtype=float,
        )
        self._logger.debug(f"Creating mat_g_2 with dimension: {mat_g_2.shape}")
        mat_g_3 = np.zeros(
            shape=(self._n_fit_globals, len(self._buffer.local_point_map)),
            dtype=float,
        )
        self._logger.debug(f"Creating mat_g_3 with dimension: {mat_g_3.shape}")

        for point_idx, entrypoint in enumerate(entrypoints):
            locals = entrypoint.localDerivs
            globals = entrypoint.globalDerivs

            # zip between locals and globals
            for global_left, local_right in zip(globals, locals):
                if (
                    global_left_mat_id := self._global_id_convert_map.get(
                        global_left.id
                    )
                ) is None:
                    continue
                mat_g_1[global_left_mat_id, local_right.id] += self._eta * (
                    global_left.value * self._buffer.beta[local_right.id]
                )
                if (
                    right_map_idx := self._buffer.local_point_map.get(
                        (local_right.id, point_idx)
                    )
                ) is not None:
                    mat_g_3[global_left_mat_id, right_map_idx] += (
                        self._eta
                        * self._buffer.beta[local_right.id]
                        * global_left.value
                    )

            # zip between globals and globals
            for global_left, global_right in zip(globals, globals):
                if (
                    (
                        global_left_mat_id := self._global_id_convert_map.get(
                            global_left.id
                        )
                    )
                    is None
                ) or (
                    (
                        right_map_idx := self._buffer.global_point_map.get(
                            (global_right.id, point_idx)
                        )
                    )
                    is None
                ):
                    continue
                right_map_idx = self._buffer.global_point_map[
                    global_right.id, point_idx
                ]
                if global_right.id == global_left.id:
                    mat_g_2[global_left_mat_id, right_map_idx] += (
                        self._eta * self._buffer.global_sum
                    )
                mat_g_2[global_left_mat_id, right_map_idx] += (
                    self._eta
                    * self._global_init_pars[global_right.id]
                    * global_right.value
                )
        return np.block(
            [
                [mat_g_1, mat_g_2, mat_g_3],
            ]
        )

    def _calculate_mat_c_1(self, entrypoints: Any):
        mat_c_1 = np.zeros(
            shape=(self._n_fit_globals, self._n_fit_globals), dtype=float
        )
        np.fill_diagonal(mat_c_1, self._lambda)
        for point_idx, entrypoint in enumerate(entrypoints):
            globals = entrypoint.globalDerivs
            for par_left, par_right in zip(globals, globals):
                if (
                    (left_id := self._global_id_convert_map.get(par_left.id))
                    is None
                ) or (
                    (right_id := self._global_id_convert_map.get(par_right.id))
                    is None
                ):
                    continue

                mat_c_1[left_id, right_id] += par_left.value * par_right.value
        return mat_c_1

    def _calculate_mat_gamma(self, entrypoints: Any):
        mat_gamma_11 = np.zeros(
            shape=(self._buffer.n_locals, self._buffer.n_locals)
        )
        np.fill_diagonal(mat_gamma_11, self._lambda)
        self._log_matrix_dimension("mat_gamma_11", mat_gamma_11)
        mat_gamma_22 = np.zeros(
            shape=(
                len(self._buffer.global_point_map),
                len(self._buffer.global_point_map),
            )
        )
        self._log_matrix_dimension("mat_gamma_22", mat_gamma_22)

        mat_gamma_33 = np.zeros(
            shape=(
                len(self._buffer.local_point_map),
                len(self._buffer.local_point_map),
            )
        )
        self._log_matrix_dimension("mat_gamma_33", mat_gamma_33)

        mat_gamma_12 = np.zeros(
            shape=(self._buffer.n_locals, len(self._buffer.global_point_map))
        )
        self._log_matrix_dimension("mat_gamma_12", mat_gamma_12)

        mat_gamma_13 = np.zeros(
            shape=(self._buffer.n_locals, len(self._buffer.local_point_map))
        )
        self._log_matrix_dimension("mat_gamma_13", mat_gamma_13)

        mat_gamma_23 = np.zeros(
            shape=(
                len(self._buffer.global_point_map),
                len(self._buffer.local_point_map),
            )
        )
        self._log_matrix_dimension("mat_gamma_23", mat_gamma_23)

        for point_idx, entrypoint in enumerate(entrypoints):
            locals = entrypoint.localDerivs
            globals = entrypoint.globalDerivs
            local_sum = sum(
                [self._buffer.beta[par.id] * par.value for par in locals]
            )

            # zip between locals and locals
            for local_left, local_right in zip(locals, locals):
                mat_gamma_11[local_left.id, local_right.id] += (
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
                        mat_gamma_33[map_idx_left, map_idx_right] += (
                            self._lambda + 1 / local_left.error**2
                        )

                    mat_gamma_33[map_idx_left, map_idx_right] += (
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
                        mat_gamma_13[local_left.id, map_idx] += (
                            self._eta * local_sum
                        )
                    mat_gamma_13[local_left.id, map_idx] += (
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
                    mat_gamma_12[local_right.id, map_idx] += (
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
                    mat_gamma_23[map_idx_global, map_idx_local] += (
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
                        mat_gamma_22[map_idx_left, map_idx_right] += (
                            self._lambda + 1 / global_left.error**2
                        )

                    mat_gamma_22[map_idx_left, map_idx_right] += (
                        self._eta
                        * self._global_init_pars[global_left.id]
                        * self._global_init_pars[global_right.id]
                    )
        mat_gamma = np.block(
            [
                [mat_gamma_11, mat_gamma_12, mat_gamma_13],
                [mat_gamma_12.T, mat_gamma_22, mat_gamma_23],
                [mat_gamma_13.T, mat_gamma_23.T, mat_gamma_33],
            ]
        )
        self._log_matrix_dimension("mat_gamma", mat_gamma)
        # with np.printoptions(precision=2, floatmode="fixed"):
        #     self._logger.debug(f"mat_gamma: \n{mat_gamma}")

        return mat_gamma

    def _scan_entry(self, entrypoints: Any, event_idx: int):
        n_local_max_id = 0
        self._buffer.global_point_map.clear()
        self._buffer.local_point_map.clear()
        self._buffer.n_locals = 0

        global_map_idx = 0
        local_map_idx = 0
        self._logger.debug(
            f"entry num: {event_idx}, num of points: {len(entrypoints)}"
        )
        for point_idx, entrypoint in enumerate(entrypoints):
            globals = entrypoint.globalDerivs
            locals = entrypoint.localDerivs
            self._buffer.global_point_map.update(
                {
                    (par.id, point_idx): global_map_idx
                    for par in globals
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
        self._logger.debug(
            f"Global point map: {self._buffer.global_point_map}"
        )
        self._logger.debug(f"Local point map: {self._buffer.local_point_map}")
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

    @property
    def entry_reader(self):
        return self._entry_reader

    @property
    def data_filename(self):
        """The data_filename property."""
        return self._data_filename

    @property
    def global_init_pars(self):
        """The global_init_pars property."""
        return self._global_init_pars

    @property
    def losses(self):
        """The losses property."""
        return self._losses

    @property
    def n_globals(self):
        """The n_globals property."""
        return self._n_globals

    @property
    def factor_mat(self):
        """The factor_mat property."""
        return self._factor_mat

    @property
    def rhs(self):
        """The rhs property."""
        return self._rhs

    @property
    def fixed_globals(self):
        """The fixed_globals property."""
        return self._fixed_globals

    @data_filename.setter
    def data_filename(self, value: str):
        self._data_filename = value

    @global_init_pars.setter
    def global_init_pars(self, value: dict[int, float]):
        self._global_init_pars = value

    @fixed_globals.setter
    def fixed_globals(self, value: list[int]):
        self._fixed_globals = value
        self._n_fit_globals = self._n_globals - len(value)
        fit_parameters = [
            x for x in range(0, self._n_globals) if x not in value
        ]
        self._global_id_convert_map = {
            global_id: par_id
            for par_id, global_id in enumerate(fit_parameters)
        }
        self._logger.info(
            f"Fixed parameter IDs: {value}. Current number of fit parameters: {self._n_fit_globals}"
        )
        self._logger.info(
            f"Global parameter ID to matrix index: {self._global_id_convert_map}"
        )

    @property
    def par_results(self):
        """The par_results property."""
        return self._par_results
