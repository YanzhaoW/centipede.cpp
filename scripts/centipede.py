import math
import os
import sys
from pathlib import Path
from typing import Any

import capnp
import numpy as np
from odrpack import odr_fit
from odrpack.result import F64Array


class Centipede:
    def __init__(
        self, schema_path: str, n_globals: int, v_lambda: float, v_eta: float
    ):
        self._entry_capnp = self._setup_schema_path(schema_path)
        self._data_filename: str = ""
        self._global_init_pars = {
            par_id: 0.0 for par_id in range(0, n_globals)
        }
        self._lambda = v_lambda
        self._eta = v_eta

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

    def analyze(self):
        with open(self._data_filename, "rb") as file:
            reader = self._entry_capnp.Entry.read_multiple_packed(file)
            for entry in reader:
                n_locals = self._scan_n_locals(entrypoints=entry.entrypoint)
                local_inits = self._local_fit(
                    entrypoints=entry.entrypoint, n_locals=n_locals
                )
                loss = self._calculate_loss(
                    entrypoints=entry.entrypoint, local_inits=local_inits
                )
                print(f"loss: {loss}. beta: {local_inits}")
                # self._analyze_entry(entry.entrypoint)

    def _local_fit(self, entrypoints: Any, n_locals: int) -> np.ndarray:
        n_points = len(entrypoints)
        x_data = np.zeros(shape=(n_locals, n_points), dtype=float)
        x_errors = np.zeros(shape=(n_locals, n_points), dtype=float)
        y_data = np.zeros(shape=n_points, dtype=float)
        for point_idx, entrypoint in enumerate(entrypoints):
            y_data[point_idx] = -sum(
                [
                    self._global_init_pars[par.id] * par.value
                    for par in entrypoint.globalDerivs
                ]
            )
            for local_deriv in entrypoint.localDerivs:
                x_data[local_deriv.id][point_idx] = local_deriv.value
                x_errors[local_deriv.id][point_idx] = local_deriv.error

        print(f"x_data: {x_data}")
        print(f"y_data: {y_data}")
        result = odr_fit(
            f=Centipede.linear_function,
            xdata=x_data,
            ydata=y_data,
            beta0=np.zeros(n_locals),
            weight_x=x_errors,
            task="implicit-ODR",
        )
        return result.beta

    def _analyze_entry(self, entrypoints: Any):
        for entrypoint in entrypoints:
            print(f"local derivs: {entrypoint.localDerivs}")
            print(f"global derivs: {entrypoint.globalDerivs}")
        pass

    def _scan_n_locals(self, entrypoints: Any):
        n_local_max_id = 0
        for entrypoint in entrypoints:
            n_local_max_id = max(
                n_local_max_id, max([par.id for par in entrypoint.localDerivs])
            )
        return n_local_max_id + 1

    def _calculate_loss(self, entrypoints: Any, local_inits: np.ndarray):
        loss = 0.0
        for entrypoint in entrypoints:
            loss = loss + sum(
                [
                    self._global_init_pars[par.id] * par.value
                    for par in entrypoint.globalDerivs
                ]
            )
            loss = loss + sum(
                [
                    local_inits[par.id] * par.value
                    for par in entrypoint.localDerivs
                ]
            )
        return loss

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
