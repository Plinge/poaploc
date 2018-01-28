import os
import errno
import shutil
import time

def make_sure_path_exists(path):
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise

def assert_empty_path(path):
    if os.path.exists(path):
        shutil.rmtree(path, True)        
        time.sleep(2)
    make_sure_path_exists(path)
    