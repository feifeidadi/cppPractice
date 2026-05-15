#include <iostream>
#include <list>

void print(const std::list<int>& lst, const std::string& note)
{
  std::cout << note << std::endl;
  for(const auto& i : lst)
  {
    std::cout << i << " ";
  }
  std::cout << std::endl;
}

int main()
{
  std::list<int> list1 {1, 10 , 55, 3, 7};
  std::list<int> list2 {8, 30 , 51, 13, 99};

  print(list1, "list1:");
  print(list2, "list2:");

  list1.sort();
  list2.sort();

  print(list1, "list1 sorted:");
  print(list2, "list2 sorted:");

  list1.merge(list2);

  print(list1, "After merging list2 into list1:");

  return 0;

}
