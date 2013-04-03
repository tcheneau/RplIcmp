from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules=[
    Extension("RplIcmp",
              ["RplIcmp.pyx", "tinyICMPlib.pxd"],
              extra_link_args=["-lcap", "caplib.o", "icmplib.o"],
             )
			      ]

setup(
  name = "RplIcmp",
  cmdclass = {"build_ext": build_ext},
  ext_modules = ext_modules,

)