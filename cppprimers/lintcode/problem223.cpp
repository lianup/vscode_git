#include <string>
#include <iostream>
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

ListNode* get_mid(ListNode *head){
	ListNode *p = head,*mid = head;
	while(p->next != NULL && p->next->next != NULL){
		p = p->next->next;
		mid = mid->next;
	}
	return mid;
}

ListNode* reverse(ListNode *head){
	ListNode *first = head,*next = head->next;
	head->next = NULL;
	while(next != NULL){
		first = next->next;
		next->next = head;
		head = next;
		next = first;
	}
//	cout << head->val << endl;
//	cout << head->next->val << endl;
	return head;

}

bool isPalindrome(ListNode *head){
	if(!head || head->next == NULL){
		return true;
	}
	ListNode *mid = get_mid(head);
	int count = 0;
	ListNode *temp = head;
	while(temp != NULL){
		temp = temp->next;
		++count;
	}
	ListNode *second = mid->next; 
	mid->next = NULL;
	ListNode *first = reverse(head);
	if(count % 2 != 0){
		first = first->next;
	}
		while(second != NULL && second != NULL){
			if(first->val != second->val){
				return false;
			}
			second = second->next;
			first = first->next;
		}
		if(second && !first || !second && first){
			return false;
		}
		return true;
}

int main(){
	ListNode *head = new ListNode(1);
	ListNode *node1 = new ListNode(1);
	ListNode *node2 = new ListNode(0);
	ListNode *node3 = new ListNode(0);
	ListNode *node4 = new ListNode(1);
	head->next = node1;
	node1->next = node2;
	node2->next = node3;
	node3->next = node4;
	cout << isPalindrome(head) << endl;
}

