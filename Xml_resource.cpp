#include "Xml_resource.h"

bool is_opening_tag(const char& first_letter, const char& second_letter) {
	if (first_letter == '<' && second_letter != '/')
		return true;
	else
		return false;
}

std::string get_value_from_line(const std::string& line, unsigned int& index) {
	std::string value;
	while (line[index] != '>') {
		value += line[index];
		++index;
	}
	return value;
}

std::vector<std::string> parse_line(const std::string& line) {
	unsigned int i = 0;
	std::vector<std::string> parsed_line;
	while (i < line.size()) {
		std::string word;
		std::string number;
		if (is_opening_tag(line[i], line[i+1])) {
			word += '<';
			++i;
			while (line[i] != '>') {
				word += line[i];
				++i;
			}
			word += '>';
			parsed_line.push_back(word);
			i += std::size("> value=");
			while (isdigit(line[i])) {
				number += line[i];
				++i;
			}
			parsed_line.push_back(number);
		}
		else {
			word += '<';
			++i;
			word += get_value_from_line(line, i);
			word += '>';
			parsed_line.push_back(word);
		}
		++i;
	}
	return parsed_line;
}

std::vector<std::string> parse_file(const std::string& file_name) {
	std::ifstream file;
	file.open(file_name);
	if (!file.is_open()) {
		throw  "Troubles with file";
	}

	std::vector<std::string> ans;

	std::string line;
	while (std::getline(file, line)) {
		std::vector<std::string> parsed_line = parse_line(line);
		for (auto& tmp : parsed_line)
			ans.push_back(tmp);
	}
	file.close();
	return ans;
}

std::unique_ptr<Node> Xml_resource::rec_load(const std::vector<std::string>& parsed, int& index) {
	std::string name = parsed[index].substr(1, parsed[index].size() - 2);
	++index;
	int value = std::stoi(parsed[index]);
	++index;
	std::vector<std::unique_ptr<Node>> children;
	while (parsed[index][1] != '/') {
		children.push_back(std::move(rec_load(parsed, index)));
	}
	++index;
	std::unique_ptr<Node> node_ptr(new Node(name, value, children));
	return node_ptr;
}

void Xml_resource::load(const std::string& file_name) {
	std::vector<std::string> parsed = parse_file(file_name);
	
	int index = 0;
	this->root = std::move(rec_load(parsed, index));
}

void Xml_resource::rec_node_print(const Node* node) const {
	std::cout << '<' << node->name << "> value=" << node->value << std::endl;
	for (unsigned int i = 0; i < size(node->children); ++i) {
		rec_node_print(node->children[i].get());
	}
	std::cout << "</" << node->name << '>' << std::endl;
}

void Xml_resource::print() const {
	rec_node_print(root.get());
}

void Xml_resource::rec_node_upload(const std::unique_ptr<Node>& node, std::ofstream& file) const  {
	file << '<' << node->name << "> value=" << node->value << std::endl;
	for (unsigned int i = 0; i < size(node->children); ++i) {
		rec_node_upload(node->children[i], file);
	}
	file << "</" << node->name << '>' << std::endl;
}

void Xml_resource::upload(const std::string& file_name) const {
	std::ofstream file;
	file.open(file_name);
	if (!file.is_open())
		throw "Trouble with file";
	rec_node_upload(this->root, file);

}

//scope of iterators

void rec_create_iterator(Node* node, std::vector<Node*>& vec) {
	vec.push_back(node);
	for (unsigned int i = 0; i < (node->children.size()); ++i) {
		Node* tmp_ptr = node->children[i].get();
		rec_create_iterator(tmp_ptr, vec);
	}
}

Xml_resource::iterator::iterator(const std::unique_ptr<Node>& node) {
	current_index = 0;
	rec_create_iterator(node.get(), this->memory);
}

Xml_resource::iterator& Xml_resource::iterator::operator++()
{
	if (current_index + 1 == memory.size())
		throw "Current index has reached its upper limit";

	++current_index;
	return *this;
}

Xml_resource::iterator& Xml_resource::iterator::operator--()
{
	if (current_index - 1 < 0)
		throw "Current index has reached its lower limit";

	--current_index;
	return *this;
}

Xml_resource::iterator Xml_resource::iterator::operator++(int) {
	Xml_resource::iterator copy = *this;
	++(*this);
	return copy;
}

Xml_resource::iterator Xml_resource::iterator::operator--(int) {
	Xml_resource::iterator copy = *this;
	--(*this);
	return copy;
}

Xml_resource::iterator& Xml_resource::iterator::operator+=(int n) {
	if (current_index + n >= memory.size())
		throw "Current index has reached its upper limit";

	current_index += n;
	return *this;
}

Xml_resource::iterator& Xml_resource::iterator::operator-=(int n) {
	if (current_index - n <= 0)
		throw "Current index has reached its lower limit";

	current_index -= n;
	return *this;
}

Xml_resource::iterator Xml_resource::find_by_name(const std::string name) const {
	Xml_resource::iterator main_it(this->begin());
	while (main_it->name != name)
		main_it++;
	return main_it;
}

Xml_resource::iterator Xml_resource::find_by_value(const int value) const {
	Xml_resource::iterator main_it(this->begin());
	while (main_it->value != value)
		main_it++;
	return main_it;
}

Xml_resource::iterator Xml_resource::add(const std::string name, const int value, Xml_resource::iterator& it) {
	Xml_resource::iterator it_from_root(this->begin());
	while (it_from_root != it)
		++it_from_root;
	if (it_from_root == it) {
		std::unique_ptr<Node> new_node(new Node(name, value));
		Xml_resource::iterator it_new_node(new_node);
		it->children.push_back(std::move(new_node));
		return it_new_node;
	}
	else
		throw "node wasn`t found";
}

bool Xml_resource::erase(Xml_resource::iterator it) {
	Xml_resource::iterator itt(this->begin());
	while (itt != it)
		++itt;
	if (itt != it)
		return false;
	else {
		Node* prev_node = &(*itt);
		--itt;
		while (itt->children[0].get() != prev_node) {
			prev_node = &(*itt);
			--itt;
		}
		for (unsigned int i = 0; i < itt->children.size(); ++i) {
			if (itt->children[i].get() == prev_node) {
				itt->children[i].release();
				itt->children.erase(itt->children.begin() + i);
				break;
			}
		}
		return true;
	}
}