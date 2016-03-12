#include <iostream>
#include "Tree23.h"

int main() {
    float inp[5] = {50.5, 10.7, 30.2, 20.2, 40.4}; 
	float fnd[5] = {50.5, 30.2, 40.4, 3.14, 2.72};
	
    Tree23<float> tr;

	for (int i=0; i<5; i++) {
		tr.insert(inp[i]);
		std::cout << inp[i] << " inserted";
		std::cout <<" (size: " << tr.size() << ") " << std::endl;
	}
	
	for (int i=0; i<5; i++) {
		if (tr.find(fnd[i])) {
			std::cout << fnd[i] << " found" << std::endl;
		} else {
			std::cout << fnd[i] << " not found" << std::endl;
		}
	}
	
	for (int i=0; i<5; i++) {
		tr.erase(fnd[i]);
		std::cout << inp[i] << " erased";		
		std::cout <<" (size: " << tr.size() << ") " << std::endl;
	}
	
	tr.clear();
	
	if (tr.empty()) {
		std::cout << "set  clear";
		std::cout << " (size: " << tr.size() << ") " << std::endl;				
	}
	
    return 0;
}        