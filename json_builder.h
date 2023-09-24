#pragma once

#include<vector>
#include<string>
#include<cstddef>
#include<stdexcept>
#include<memory>

#include "json.h"

namespace json {

	class Builder;

	class DictItemContext;
	class KeyItemContext;
	class KeyValueContext;

	class ArrayItemContext;
	class ArrayValueContext;

	class BaseContext {
	public:

		BaseContext(Builder& builder);
		
		KeyItemContext Key(std::string);
		Builder& Value(Node::Value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Node  Build();

	private:
		Builder& base;

	};

	class DictItemContext : public BaseContext {
	public:
		DictItemContext(BaseContext base) : BaseContext(base) {}
		//Supported: Key, EndDict.

		Builder& Value(Node::Value) = delete;
		DictItemContext StartDict() = delete;
		Builder& StartArray() = delete;
		Builder& EndArray() = delete;
		Node  Build() = delete;
	};

	class KeyItemContext : public BaseContext {
	public:
		KeyItemContext(BaseContext base) : BaseContext(base) {}
		//Supported: Value, StartDict, StartArray.

		KeyValueContext Value(Node::Value);//<------------

		KeyItemContext Key(std::string) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		Node  Build() = delete;

	};

	class KeyValueContext : public BaseContext {
	public:
		KeyValueContext (BaseContext base) : BaseContext(base) {}
		//Supported: Key, EndDict.

		Builder& Value(Node::Value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
		Node  Build() = delete;
	};


	class ArrayItemContext : public BaseContext {
	public:
		ArrayItemContext(BaseContext base) : BaseContext(base) {}
		//Supported: Value, StartDict, StartArray, EndArray.

		ArrayValueContext Value(Node::Value value);

		KeyItemContext Key(std::string) = delete;
		Builder& EndDict() = delete;
		Node  Build() = delete;
	};


	class ArrayValueContext : public BaseContext {
	public:
		ArrayValueContext(BaseContext base) : BaseContext(base) {}
		//Supported: Value, StartDict, StartArray, EndArray.

		ArrayValueContext Value(Node::Value value);

		KeyItemContext Key(std::string) = delete;
		Builder& EndDict() = delete;
		Node  Build() = delete; 

	};


	class Builder {
	public:

		KeyItemContext  Key(std::string);

		Builder&  Value(Node::Value);

		DictItemContext StartDict();

		ArrayItemContext StartArray();

		Builder&  EndDict();

		Builder&  EndArray();

		Node  Build();

	private:
		Node root_;
		std::vector<std::unique_ptr <Node>> nodes_stack_;
		//-----------------------------------------------
		Node GetNode(Node::Value& value);
		void AddNode(Node node);
	};

	



}//namespace json

