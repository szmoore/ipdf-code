
import sys
import os
import time
import subprocess
from progressbar import * # From ipython github site
from IPython.core.display import Image
from IPython.display import display, clear_output
import threading

options = {
    "real_names" : ["single", "double", "long", "virtual", "Rational_GMPint", "Rational_Arbint", "mpfrc++", "iRRAM", "ParanoidNumber", "GMPrat"],
    "ipdf_src" : "../src/",
    "ipdf_bin" : "../bin/ipdf",
    "local_bin" : "./",
    "tests" : "../src/tests/",
    "ignore" : ["virtual", "Rational_Arbint", "iRRAM", "ParanoidNumber"],
    "tobuild" : ["single", "double", "mpfrc++","GMPrat"],
    "numerical_tests" : ["identitytests"],
    "built" : []
}
