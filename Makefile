all:

	rm -f index.bin
	g++ -o db_engine driver.cpp c_api.cpp BPlusTree.cpp DiskManager.cpp
	@echo "seq input file is this :"
	python3 input_seq.py

	@echo "random inpt file is this :"
	python3 input_random.py


clean:

	rm -f db_engine index.bin random_input.txt sequential_input.txt

