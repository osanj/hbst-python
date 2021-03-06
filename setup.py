import os
import subprocess
import sys
import shutil

from setuptools import Command, Extension, setup
from setuptools.command.build_ext import build_ext


NAME = "hbst-python"
VERSION = "0.1.0"
DEBUG = False


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
        for d in ("build", "dist", NAME.replace("-", "_"), "wheelhouse"):
            if os.path.exists(d):
                shutil.rmtree(d)


setup(
    name=NAME,
    version=VERSION,
    author="Jonas Schuepfer",
    author_email="jonasschuepfer@gmail.com",
    description="Python bindings for hbst library for visual place recognition",
    long_description=("Python bindings for the original implementation of the hamming distance embedding binary "
                      "search tree for feature-based visual place recognition implementation, "
                      "for more details see https://gitlab.com/srrg-software/srrg_hbst"),
    project_urls={"Source Code": "https://github.com/osanj/hbst-python"},
    license="BSD",
    platforms=["Linux"],
    ext_modules=[CMakeExtension(NAME)],
    cmdclass={
        "build_ext": CMakeBuild,
        "clean": CleanCommand
    },
    license_files=("LICENSE.txt",),
    install_requires=["numpy"],
    zip_safe=False,
    python_requires=">=3.6",
)
