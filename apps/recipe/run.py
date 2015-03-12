import time, subprocess

start = time.time()
subprocess.call(["./extract_recipe","../../images/input_image.jpg","50","1","1"])
end = time.time()
elapsed = end-start
print elapsed
