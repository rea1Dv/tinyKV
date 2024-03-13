#include <iostream>
#include "skiplist.h"

int main() {
  skip_list::SkipList<int, std::string> skip_list(6);
  // key use int, if use other type, need define compare func
  // if want to modify type of key, modify LoadFile func as well
  skip_list.InsertElement(1, "cpp"); 
  skip_list.InsertElement(3, "c#"); 
  skip_list.InsertElement(7, "c"); 
  skip_list.InsertElement(8, "python"); 
  skip_list.InsertElement(9, "rust"); 
  skip_list.InsertElement(19, "java"); 
  skip_list.InsertElement(19, "golang"); 

  std::cout << "skip_list size:" << skip_list.Size() << std::endl;

  skip_list.DumpFile();

  //skip_list.LoadFile();

  skip_list.SearchElement(9);
  skip_list.SearchElement(18);

  skip_list.PrintList();

  skip_list.DeleteElement(3);
  skip_list.DeleteElement(7);

  std::cout << "skip_list size:" << skip_list.Size() << std::endl;

  skip_list.PrintList();
}
