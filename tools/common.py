
import sys
import os
import time
import subprocess
from progressbar import * # From ipython github site
from IPython.core.display import Image
from IPython.display import display, clear_output
import threading

options = {
    "real_names" : ["float", "double", "long", "virtual", "Rational_Gmpint", "Rational_Arbint", "mpfr", "iRRAM", "ParanoidNumber", "Gmprat"],
    "ipdf_src" : "../src/",
    "ipdf_bin" : "../bin/ipdf",
    "local_bin" : "./",
    "tests" : "../src/tests/",
    "ignore" : ["virtual", "Rational_Arbint", "iRRAM", "ParanoidNumber"],
    "tobuild" : ["float", "double", "mpfr-1024","Gmprat", "path-Gmprat", "path-mpfr-1025"],
    "numerical_tests" : ["identitytests"],
    "built" : []
}
