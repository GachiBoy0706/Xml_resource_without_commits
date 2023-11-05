#include "Xml_resource.h"

int main() {

	std::unique_ptr<Xml_resource> tree = std::move(Xml_resource::create());
	try {
		tree->load("file.txt");
		tree->print();
		std::cout << std::endl;
	}
	catch (const char* error_message)
	{
		std::cout << error_message;
	}

	Xml_resource::iterator it1(tree->begin());
	it1++;
	Xml_resource::iterator it2(it1);
	std::cout << it1->value << std::endl;
	it1 += 2;
	std::cout << it1->value << std::endl;

	std::cout << (it1 == it2) << std::endl;
	std::cout << std::endl;
	try
	{
		Xml_resource::iterator it_child = tree->add("CHILD_ADDED", 6666, it2);
		std::cout << it_child->value << std::endl;
		std::cout << std::endl;
		tree->print();
		std::cout << std::endl;
		tree->upload("file_output.txt");
		std::cout << std::endl;
		std::cout << tree->erase(it_child) << std::endl;
		tree->print();
	}
	catch (const char* error_message)
	{
		std::cout << error_message << std::endl;
	}
	return 0;
}