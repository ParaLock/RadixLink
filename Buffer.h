#pragma once

#include <vector>

struct Buffer {
	
	Buffer() {

		isLast = false;
		seq = 0;
	}

	~Buffer() {	}

	void write(char* src, size_t size, bool incPos = true) {
		
		unsigned int count = 0;
		
		for(int i = 0; i < size; i++) {
		
			data.push_back(src[count]);
			
			if(incPos)
				count++;
		}
	}
	
	size_t getSize() {
		
		return data.size();
	}
	
	char * getBase() {
		
		return &data[0];
	}
	
	char* getCurrentOffset() {
	
		return &data[0] + data.size();
	}
	
	void print() {

		for(int i = 0; i < data.size(); i++) {

			std::cout << data[i];
		}
		
		std::cout << std::endl;
	}
	
	bool         	  isLast;
	unsigned int 	  seq;
	size_t       	  size;
	std::vector<char> data;
};
