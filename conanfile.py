# pylint: disable-all

import os

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain


class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("spdlog/1.16.0", options={"use_std_fmt": True, "no_exceptions": True})  # type: ignore
        self.requires("magic_enum/0.9.7")  # type: ignore
        self.requires("cli11/2.6.0")  # type: ignore

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = ""
        tc.generate()
