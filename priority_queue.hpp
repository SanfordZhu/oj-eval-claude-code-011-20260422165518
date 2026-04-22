#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
	struct Node {
		T data;
		Node *left;
		Node *right;
		int dist;
		Node(const T &d) : data(d), left(nullptr), right(nullptr), dist(0) {}
	};

	Node *root;
	size_t sz;
	Compare cmp;

	// Merge two leftist heaps. Exception-safe: if Compare throws,
	// both heaps are restored to their original state.
	Node *merge(Node *a, Node *b) {
		if (!a) return b;
		if (!b) return a;
		// Compare might throw, but nothing has been modified yet.
		if (cmp(a->data, b->data)) {
			std::swap(a, b);
		}
		// a is the new root (a->data >= b->data)
		Node *oldRight = a->right;
		try {
			a->right = merge(a->right, b);
		} catch (...) {
			a->right = oldRight;
			throw;
		}
		// Maintain leftist property: left child must have dist >= right child's dist
		if (!a->left || (a->right && a->right->dist > a->left->dist)) {
			std::swap(a->left, a->right);
		}
		a->dist = a->right ? (a->right->dist + 1) : 0;
		return a;
	}

	void clear(Node *n) {
		if (!n) return;
		clear(n->left);
		clear(n->right);
		delete n;
	}

	Node *clone(Node *n) {
		if (!n) return nullptr;
		Node *newNode = new Node(n->data);
		try {
			newNode->left = clone(n->left);
			newNode->right = clone(n->right);
		} catch (...) {
			clear(newNode);
			throw;
		}
		newNode->dist = n->dist;
		return newNode;
	}

public:
	priority_queue() : root(nullptr), sz(0) {}

	priority_queue(const priority_queue &other) : root(nullptr), sz(other.sz) {
		if (other.root) {
			root = clone(other.root);
		}
	}

	~priority_queue() {
		clear(root);
	}

	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		clear(root);
		root = nullptr;
		sz = 0;
		if (other.root) {
			root = clone(other.root);
		}
		sz = other.sz;
		return *this;
	}

	const T &top() const {
		if (!root) throw container_is_empty();
		return root->data;
	}

	void push(const T &e) {
		Node *newNode = new Node(e);
		++sz;
		try {
			root = merge(root, newNode);
		} catch (...) {
			delete newNode;
			--sz;
			throw;
		}
	}

	void pop() {
		if (!root) throw container_is_empty();
		Node *oldRoot = root;
		Node *left = root->left;
		Node *right = root->right;
		try {
			root = merge(left, right);
		} catch (...) {
			root = oldRoot;
			throw;
		}
		delete oldRoot;
		--sz;
	}

	size_t size() const {
		return sz;
	}

	bool empty() const {
		return sz == 0;
	}

	void merge(priority_queue &other) {
		if (this == &other) return;
		try {
			root = merge(root, other.root);
		} catch (...) {
			throw;
		}
		other.root = nullptr;
		sz += other.sz;
		other.sz = 0;
	}
};

}

#endif
