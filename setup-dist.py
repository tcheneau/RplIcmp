from distutils.core import setup
from distutils.extension import Extension


ext_modules=[
    Extension("RplIcmp",
              sources=["RplIcmp.c"],
              extra_link_args=["-lcap", "caplib.o", "icmplib.o"],
             )
			]

setup(
  name = "RplIcmp",
  ext_modules = ext_modules,

)