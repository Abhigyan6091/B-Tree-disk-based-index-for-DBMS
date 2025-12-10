
import random

FILENAME = "sequential_input.txt"

NUM_RECORDS = 100000

print(f"Generating {FILENAME} with {NUM_RECORDS} records...")

with open(FILENAME, "w") as f:

    keys = list(range(NUM_RECORDS))
    random.shuffle(keys)


    for i in range(NUM_RECORDS):
        val = f"RND_DATA_{i}_" + "R" * (80 - len(str(i)))
        f.write(f"INSERT {i} {val}\n")



    for i in range(NUM_RECORDS):
        f.write(f"READ {i}\n")



    for _ in range(NUM_RECORDS):
        target = random.randint(0, NUM_RECORDS - 1)
        f.write(f"DELETE {target}\n")


    f.write(f"RANGE 0 {NUM_RECORDS} {NUM_RECORDS}\n")

print("Done! File generated.")

