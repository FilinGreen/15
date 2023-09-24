#include "json_builder.h"



namespace json {

	
//------------------------------------------------Context------------------------------------------------
	BaseContext::BaseContext(Builder& builder) :base(builder) {}

	KeyItemContext BaseContext::Key(std::string key) {
		return base.Key(std::move(key));
	}

	Builder& BaseContext::Value(Node::Value value) {
		return base.Value(std::move(value));
	}

	DictItemContext BaseContext::StartDict() {
		return  base.StartDict();
	}

	ArrayItemContext BaseContext::StartArray() {
		return base.StartArray();
	}

	Builder& BaseContext::EndDict() {
		return base.EndDict();
	}

	Builder& BaseContext::EndArray() {
		return base.EndArray();
	}

	Node  BaseContext::Build() {
		return base.Build();
	}

	//----------------KeyValueContext--------

	KeyValueContext KeyItemContext::Value(Node::Value value) {
		return BaseContext(BaseContext::Value(std::move(value)));
	}
	
	ArrayValueContext ArrayItemContext::Value(Node::Value value) {
		return BaseContext(BaseContext::Value(std::move(value)));
	}

	ArrayValueContext ArrayValueContext::Value(Node::Value value) {
		return BaseContext(BaseContext::Value(std::move(value)));
	}





	//------------------------------------------------Builder------------------------------------------------

	KeyItemContext Builder::Key(std::string key) {

		if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict() || !root_.IsNull()) {
			throw std::logic_error("Wrong key addition");
		}

		nodes_stack_.emplace_back(std::make_unique<Node>(std::move(key)));

		return BaseContext(*this);
	}

	Builder& Builder::Value(Node::Value value) {	
		if (!root_.IsNull()) {
			throw std::logic_error("Wrong value addition");
		}

		nodes_stack_.emplace_back(std::make_unique<Node>(GetNode(value)));
		AddNode(*nodes_stack_.back());
	 
			return *this;
	}
	
	DictItemContext Builder::StartDict() {
		if (!root_.IsNull()) {
			throw std::logic_error("Wrong Dict addition");
		}
		nodes_stack_.push_back(std::unique_ptr <Node>(new Node(Dict{})));
		return BaseContext(*this);
	}

	ArrayItemContext Builder::StartArray() {
		if (!root_.IsNull()) {
			throw std::logic_error("Wrong Array addition");
		}
		nodes_stack_.push_back(std::unique_ptr <Node>(new Node({std::vector<Node>()})));
		return BaseContext(*this);
	}

	Builder& Builder::EndDict() {
		if (nodes_stack_.empty() || !root_.IsNull() || nodes_stack_.back()->IsArray()) {
			throw std::logic_error("Try to end non-existent dict");
		}
		AddNode(*nodes_stack_.back().release());
		return *this;
	}

	Builder& Builder::EndArray() {
		if (nodes_stack_.empty() || !root_.IsNull() || nodes_stack_.back()->IsDict()) {
			throw std::logic_error("Try to end non-existent array");
		}
		AddNode(*nodes_stack_.back().release());
		return *this;
	}

	Node  Builder::Build() {
		if (!nodes_stack_.empty()||root_.IsNull()) {
			throw std::logic_error("Try to build uncomplete node");
		}
		return root_;
	}

	Node Builder::GetNode(Node::Value& value) {

		if (std::holds_alternative<std::string>(value)) {
			return { std::get<std::string>(value) };
		}
		if (std::holds_alternative<int>(value)) {
			return { std::get<int>(value) };
		}
		if (std::holds_alternative<double>(value)) {
			return { std::get<double>(value) };
		}
		if (std::holds_alternative<bool>(value)) {
			return { std::get<bool>(value) };
		}
		if (std::holds_alternative<Dict>(value)) {
			return { std::get<Dict>(value) };
		}
		if (std::holds_alternative<Array>(value)) {
			return { std::get<Array>(value) };
		}
		return {};
	}

	void Builder::AddNode(Node node) {
		nodes_stack_.pop_back();
		if (nodes_stack_.empty()) {
			root_ = std::move(node);
		}
		else if (nodes_stack_.back()->IsArray()) {//Если до этого создавался вектор
			std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(node));
		}
		else if (nodes_stack_.back()->IsString()) {//Если до этого была строка 
			std::string key = nodes_stack_.back()->AsString();
			nodes_stack_.pop_back();
			std::get<Dict>(nodes_stack_.back()->GetValue()).emplace(key, std::move(node));
		}
		else {

			throw std::logic_error("Wrong node addition");
		}
}

	

}//namespace json
