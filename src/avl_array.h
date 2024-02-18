#ifndef INCLUDE_AVL_H
#define INCLUDE_AVL_H
#include <iostream>
#include <cassert>
using namespace std;

#define LEFT_BAL 1
#define RIGHT_BAL -1

template <typename K, typename V>
class AVL
{
public:
  class node
  {
  public:
    K key;
    V value;
    V sum;
    int height;
    node *left;
    node *right;

    node(K _key, V _value)
    {
      height = 1;
      key = _key;
      value = _value;
      sum = _value;
      left = NULL;
      right = NULL;
    }

    // void check(){
    //   V check_sum = value;
    //   if(left){
    //     check_sum += left->sum;
    //   }
    //   if(right){
    //     check_sum += right->sum;
    //   }
    //   assert(check_sum == sum);
    // }

    V no_left_sum()
    {
      if (left)
      {
        return sum - left->sum;
      }
      return sum;
    }
    void maintain()
    {
      sum = value;
      if (left)
      {
        sum += left->sum;
      }
      if (right)
      {
        sum += right->sum;
      }
    }
  };
  node *root = NULL;
  node *free_node = nullptr;
  node *most_right_node = nullptr;
  int n;
  void insert(K k, V v)
  {
    root = insertUtil(root, k, v);
  }

  void append(K k, V v)
  {
    root = appendUtil(root, k, v);
  }

  void remove(K k)
  {
    root = removeUtil(root, k);
  }
  node *search(K x)
  {
    return searchUtil(root, x);
  }
  void inorder()
  {
    inorderUtil(root);
    cout << endl;
  }

  V getDistanceAndRemovel(K key, V size)
  {
    V distance = 0;
    root = innerGetDistanceAndRemovel(root, key, size, distance);
    return distance;
  }

  node *innerGetDistanceAndRemovel(node *head, K key, V size, V &distance)
  {

    if (key < head->key)
    {
      distance += head->no_left_sum();
      head->sum -= size;
      head->left = innerGetDistanceAndRemovel(head->left, key, size, distance);
    }
    else if (key > head->key)
    {
      head->right = innerGetDistanceAndRemovel(head->right, key, size, distance);
      head->sum -= size;
    }
    else
    {
      distance += head->no_left_sum();

      node *r = head->right;
      if (head->right == NULL)
      {
        node *l = head->left;
        if (free_node)
        {
          delete (head);
        }
        else
        {
          free_node = head;
        }
        head = l;
      }
      else if (head->left == NULL)
      {
        if (free_node)
        {
          delete (head);
        }
        else
        {
          free_node = head;
        }
        head = r;
      }
      else
      {
        while (r->left != NULL)
          r = r->left;
        head->key = r->key;
        head->value = r->value;

        head->right = removeUtil(head->right, r->key);
        head->maintain();
      }
    }
    if (head == NULL)
      return head;

    head->height = 1 + max(height(head->left), height(head->right));
    int bal = height(head->left) - height(head->right);
    if (bal > LEFT_BAL)
    {
      if (height(head->left) >= height(head->right))
      {
        return rightRotation(head);
      }
      else
      {
        head->left = leftRotation(head->left);
        return rightRotation(head);
      }
    }
    else if (bal < RIGHT_BAL)
    {
      if (height(head->right) >= height(head->left))
      {
        return leftRotation(head);
      }
      else
      {
        head->right = rightRotation(head->right);
        return leftRotation(head);
      }
    }
    return head;
  }

  V getDistance(K key)
  {
    V distance = 0;
    for (node *head = root; head != nullptr;)
    {
      // head->check();
      if (key < head->key)
      {
        distance += head->no_left_sum();
        // distance += head->value;
        head = head->left;
      }
      else if (key == head->key)
      {
        distance += head->no_left_sum();
        // distance += head->value;
        return distance;
      }
      else
      {
        head = head->right;
      }
    }
    return 0;
  }

private:
  int height(node *head)
  {
    if (head == NULL)
      return 0;
    return head->height;
  }

  node *rightRotation(node *head)
  {
    node *newhead = head->left;
    head->left = newhead->right;
    newhead->right = head;
    head->height = 1 + max(height(head->left), height(head->right));
    newhead->height = 1 + max(height(newhead->left), height(newhead->right));

    head->maintain();
    newhead->maintain();

    return newhead;
  }

  node *leftRotation(node *head)
  {
    node *newhead = head->right;
    head->right = newhead->left;
    newhead->left = head;
    head->height = 1 + max(height(head->left), height(head->right));
    newhead->height = 1 + max(height(newhead->left), height(newhead->right));

    head->maintain();
    newhead->maintain();

    return newhead;
  }

  void inorderUtil(node *head)
  {
    if (head == NULL)
      return;
    inorderUtil(head->left);
    cout << head->key << " ";
    inorderUtil(head->right);
  }

  node *appendUtil(node *head, K k, V v)
  {
    if (head == NULL)
    {
      n += 1;
      node *temp = nullptr;
      if (free_node)
      {
        temp = new (free_node) node(k, v);
        free_node = nullptr;
      }
      else
      {
        temp = new node(k, v);
      }
      most_right_node = temp;
      return temp;
    }
    else
    {
      head->sum += v;
      head->right = appendUtil(head->right, k, v);
    }

    head->height = 1 + max(height(head->left), height(head->right));
    int bal = height(head->left) - height(head->right);
    // if (bal > LEFT_BAL)
    // {
    //   assert(0);

    //   // if (k < head->left->key)
    //   // {
    //   //   assert(0);
    //   //   return rightRotation(head);
    //   // }
    //   // else
    //   {
    //     head->left = leftRotation(head->left);
    //     return rightRotation(head);
    //   }
    // }
    // else
    if (bal < RIGHT_BAL)
    {
      if (k > head->right->key)
      {
        return leftRotation(head);
      }
      // else
      // {
      //   assert(0);
      //   head->right = rightRotation(head->right);
      //   return leftRotation(head);
      // }
    }
    return head;
  }

  node *insertUtil(node *head, K k, V v)
  {
    if (head == NULL)
    {
      n += 1;
      node *temp = nullptr;
      if (free_node)
      {
        temp = new (free_node) node(k, v);
        free_node = nullptr;
      }
      else
      {
        temp = new node(k, v);
      }
      most_right_node = temp;
      return temp;
    }
    if (k < head->key)
    {
      head->sum += v;
      head->left = insertUtil(head->left, k, v);
    }
    else if (k > head->key)
    {
      head->sum += v;
      head->right = insertUtil(head->right, k, v);
    }
    else
    {
      assert(0);
    }

    head->height = 1 + max(height(head->left), height(head->right));
    int bal = height(head->left) - height(head->right);
    if (bal > LEFT_BAL)
    {
      if (k < head->left->key)
      {
        return rightRotation(head);
      }
      else
      {
        head->left = leftRotation(head->left);
        return rightRotation(head);
      }
    }
    else if (bal < RIGHT_BAL)
    {
      if (k > head->right->key)
      {
        return leftRotation(head);
      }
      else
      {
        head->right = rightRotation(head->right);
        return leftRotation(head);
      }
    }
    return head;
  }
  node *removeUtil(node *head, K k)
  {
    if (head == NULL)
      return NULL;
    if (k < head->key)
    {
      head->left = removeUtil(head->left, k);
      head->maintain();
    }
    else if (k > head->key)
    {
      head->right = removeUtil(head->right, k);
      head->maintain();
    }
    else
    {
      node *r = head->right;
      if (head->right == NULL)
      {
        node *l = head->left;
        if (free_node)
        {
          delete (head);
        }
        else
        {
          free_node = head;
        }
        head = l;
      }
      else if (head->left == NULL)
      {
        if (free_node)
        {
          delete (head);
        }
        else
        {
          free_node = head;
        }
        head = r;
      }
      else
      {
        while (r->left != NULL)
          r = r->left;
        head->key = r->key;
        head->value = r->value;

        head->right = removeUtil(head->right, r->key);
        head->maintain();
      }
    }
    if (head == NULL)
      return head;
    head->height = 1 + max(height(head->left), height(head->right));
    int bal = height(head->left) - height(head->right);
    if (bal > LEFT_BAL)
    {
      if (height(head->left) >= height(head->right))
      {
        return rightRotation(head);
      }
      else
      {
        head->left = leftRotation(head->left);
        return rightRotation(head);
      }
    }
    else if (bal < RIGHT_BAL)
    {
      if (height(head->right) >= height(head->left))
      {
        return leftRotation(head);
      }
      else
      {
        head->right = rightRotation(head->right);
        return leftRotation(head);
      }
    }
    return head;
  }
  node *searchUtil(node *head, K x)
  {
    if (head == NULL)
      return NULL;
    K k = head->key;
    if (k == x)
      return head;
    if (k > x)
      return searchUtil(head->left, x);
    if (k < x)
      return searchUtil(head->right, x);
  }
};

/*
int main(){
    AVL<float> t;
    t.insert(1.3);
    t.insert(2.4);
    t.insert(3.5);
    t.insert(4.6);
    t.insert(5.7);
    t.insert(6.8);
    t.insert(7.9);
    t.inorder();
    t.remove(5.7);
    t.remove(6.8);
    t.remove(7.9);
    t.inorder();
}
*/

#endif