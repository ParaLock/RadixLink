#pragma once

#include <vector>
#include <iostream>

struct Buffer {
	
	Buffer() {

		isLast = false;
		seq = 0;
		//count = 0;
	}

	Buffer(size_t size) : data(size) {

		isLast = false;
		seq    = 0;
		lastOffset = 0;
		//count = 0;
		//count = size;
	}

	~Buffer() {	}

	unsigned int readSeq(char* dest, unsigned int size) {

		unsigned int count = 0;

		if(size > data.size() - lastOffset) {

			count = data.size() - lastOffset;
		
		} else {
			
			count = size;
		}

		memcpy(dest, getBase() + lastOffset, count);
		lastOffset += count;

		return count;
	}

	unsigned int read(char* dest, unsigned int size, unsigned int offset = 0) {

		unsigned int count = 0;

		if(size > data.size()) {

			count = data.size();
		
		} else {
			
			count = size;
		}

		memcpy(dest, getBase() + offset, count);

		return count;
	}

	void write(char* src, unsigned int size, bool incPos = true) {
		
		unsigned int count = 0;
		// data.resize(data.size() + size);

		// memcpy(getBase() + count, src, size);
		// count += size;

		for(int i = 0; i < size; i++) {
		
			data.push_back(src[count]);
			
			if(incPos)
				count++;
		}
	}
	
	unsigned int getCurrOffset() {

		return lastOffset;
	}

	unsigned int getSize() {
		
		return data.size();
	}
	
	char * getBase() {
		
		if(data.size() == 0) {
			return nullptr;
		}

		return &data[0];
	}
	
	void clear() {

		data.clear();
		lastOffset = 0;
	}

	void resize(size_t num) {

		data.resize(num);
	}

	char* getCurrentOffsetPtr() {
	
		return &data[0] + data.size();
	}
	
	void print() {

		for(int i = 0; i < data.size(); i++) {

			std::cout << data[i];
		}
		
		std::cout << std::endl;
	}
	
	bool         	  isLast;
	unsigned int      lastOffset;
	unsigned int 	  seq;
	//size_t  		  count;
	//size_t       	  size;
	std::vector<char> data;
};
