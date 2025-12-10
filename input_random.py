import random


FILENAME = "random_input.txt"

NUM_RECORDS = 500000


print(f"Generating {FILENAME} with {NUM_RECORDS} records (Randomly Interleaved)...")



operations = []


for i in range(NUM_RECORDS):
    val = f"RND_DATA_{i}_" + "R" * (80 - len(str(i)))
    operations.append(f"INSERT {i} {val}")

for i in range(NUM_RECORDS):
    operations.append(f"READ {i}")

for i in range(NUM_RECORDS):
    operations.append(f"DELETE {i}")



random.shuffle(operations)

with open(FILENAME, "w") as f:

    for op in operations:

        f.write(op + "\n")

    f.write(f"RANGE 0 {NUM_RECORDS} {NUM_RECORDS}\n")

print(f"Done! {len(operations) + 1} lines written to {FILENAME}.")

