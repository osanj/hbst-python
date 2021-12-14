import os
import re
import subprocess
import sys
import shutil

from setuptools import Command, Extension, setup
from setuptools.command.build_ext import build_ext


NAME = "hbst-python"
VERSION = "0.0.1"
DEBUG = True


class CMakeExtension(Extension):

    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        # required for auto-detection & inclusion of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        debug = int(os.environ.get("DEBUG", DEBUG))
        cfg = "Debug" if debug else "Release"
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE={cfg}",  # not used on MSVC, but no harm
            f"-DVERSION_INFO={self.distribution.get_version()}",
        ]
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        build_args = []
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                build_args += [f"-j{self.parallel}"]

        print(self.build_temp)
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp
        )
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, cwd=self.build_temp
        )


class CleanCommand(Command):
    """Custom clean command to tidy up the project root"""

    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        for d in ("build", "dist", NAME.replace("-", "_"))
            if os.path.exists(d):
                shutil.rmtree(d)


# The information here can also be placed in setup.cfg - better separation of
# logic and declaration, and simpler if you include description/version in a file.
setup(
    name="hbst-python",
    version="0.0.1",
    author="Jonas Schuepfer",
    author_email="jonasschuepfergmail.com",
    description="Python bindings for the original Hamming Binary Search Tree implementation",
    long_description="",
    ext_modules=[CMakeExtension("hbst-python")],
    cmdclass={
        "build_ext": CMakeBuild,
        "clean": CleanCommand
    },
    zip_safe=False,
    extras_require={"test": ["pytest>=6.0"]},
    python_requires=">=3.6",
)