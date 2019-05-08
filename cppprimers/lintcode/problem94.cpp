#include <iostream>
#include <vector>
#include <string>
#include <limits.h>
using namespace std;

class TreeNode{
public:
	int val;
	TreeNode *left;
	TreeNode *right;
	TreeNode(int val){
		this->val = val;
		this->left = NULL;
		this->right = NULL;
	}
};

int max_val = INT_MIN;

/**
 * 正确做法：比较root->val,root->val + left,root->val + right,roo->val + left + rigtht得到局部最优解（此值不可向上传）
 * 比较root->val,root->val + left,root->val + right得到最优解，然后往上传
 */
int getMax2(TreeNode *node){

	if(node == NULL){
		return 0;
	}
	int left = getMax(node->left),right = getMax(node->right);
	int lAndR = left + right + node->val;
	int maxR = max(left,right);
	maxR += node->val;
	maxR = max(maxR,node->val);
	max_val = max(max_val,maxR);
	max_val = max(max_val,lAndR);
	return maxR;

}


int maxPathSum2(TReeNode *root){
	return max(getMax2(root),max_val);
} 

/**
 * 求经过某个节点的最大路径
 */
int getMax(TreeNode *node){
	if(node == NULL){
		return 0;
	}
	int left = getMax(node->left),right = getMax(node->right);
	int lAndR = left + right;
	int maxR = left > right ? left : right;
	maxR += node->val;
	maxR = max(maxR,lAndR+node->val);
	return max(node->val,maxR);
}
/**
 * 我的想法：求出每个节点的最大路径，然后找到最大的返回；但是这个有问题,lefR+rightR+node->val这个为局部最优解，
 * 不能往上传。
 * 例子：{1,2,-5,4,#,5,6}得到13，结果应该为8才对
 */

int maxPathSum(TreeNode * root) {
	if(root == NULL){
		return 0;
	}
	int res = getMax(root);
	int leftR = INT_MIN,rightR = INT_MIN;
	if(root->left != NULL){
		leftR = maxPathSum(root->left);
	}
	if(root->right != NULL){
		rightR = maxPathSum(root->right);
	}
	return max(res,max(leftR,rightR));
}


int main(){

	TreeNode root(1);
	TreeNode left(2);
	TreeNode right(-5);
	TreeNode node4(4);
	TreeNode node6(5);
	TreeNode node7(6);
	root.left = &left;
	root.right = &right;
	left.left = &node4;
	left.right = NULL;
	right.left = &node6;
	right.right = &node7;
	TreeNode *node = &root;
	cout << maxPathSum(node) << endl;
	return 0;
}
