import subprocess, time

start = time.time()
ret = subprocess.call(['./extract_recipe', '../../images/input_image.jpg', '50','0'])
end = time.time()
elp =end-start

print elp
