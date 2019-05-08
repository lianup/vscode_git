#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;
class ListNode;
class ListNode{
public:
	int val;
	ListNode *next;
	ListNode(int val){
		this->val = val;
		this->next = NULL;
	}
};

ListNode* getMid(ListNode *root){
	ListNode *cur = root, *next = root;
	while(next->next != NULL && next->next->next != NULL){
		cur = cur->next;
		next = next->next->next;
	}
	return cur;
}


ListNode* reverse(ListNode *root){
	if(root->next == NULL){
		return root;
	}
	ListNode *second = root->next,*third = second->next;
	root->next = NULL;
	while(second != NULL){
		second->next = root;
		root = second;
		second = third;
		if(third == NULL){
			return root;
		}
		third = third->next;
	}
	return root;
}


/**
 * problem 99
 * 思路：原地改变链表，把后半部分reverse，然后再把前面和后面结合起来
 */
void reorderList(ListNode * head) {
	if(head == NULL || head->next == NULL){
		return;
	}
	ListNode *mid = getMid(head);
	ListNode *second = mid->next;
	mid->next = NULL;
	second = reverse(second);
	ListNode *tempA = head, *tempB = second;
	ListNode *next = tempA;
	tempA = tempA->next;
	while(tempA != NULL && tempB != NULL){
		next->next = tempB;
		tempB = tempB->next;
		next = next->next;
		next->next = tempA;
		tempA = tempA->next;
		next = next->next;
	}
	if(tempA != NULL){
		next->next = tempA;
	}else if(tempB != NULL){
		next->next = tempB;
	}
}


int main(){

	ListNode root(1),node1(2),node2(3),node3(4),node4(5);
	root.next = &node1;
//	node1.next = &node2;
//	node2.next = &node3;
//	node3.next = &node4;
	ListNode *head = &root;
	reorderList(head);
	while(head != NULL){
		cout << head->val << endl;
		head = head->next;
	}
	return 0;
}


