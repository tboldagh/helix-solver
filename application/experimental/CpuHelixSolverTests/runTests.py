import os
import json
import subprocess

with open("/helix/repo/test_results/test_list.txt", "r") as file:
    configs = file.readlines()
    configs = [config.strip() for config in configs]

for config in configs:
    cmd = ["/helix/repo/build/application/experimental/CpuHelixSolverTests/CpuHelixSolverTests", "--config", config]
    print(f"Running command: {' '.join(cmd)}")
    print("----------------------------------------------------------------")
    result = subprocess.run(cmd, capture_output=False)
    print("----------------------------------------------------------------")
    if result.returncode != 0:
        print(f"Test failed with return code {result.returncode}")
    else:
        print("Test completed successfully")
    print("\n\n")
