// This is an example of C++ singleton
// Compile: g++ -std=c++17 -Wall -Wextra -O2 singleton.C -o singleton

#include <iostream>

class singletonExample
{
  public:

  static singletonExample& getInstance()
  {
    static singletonExample s;
    return s;
  }

  void doSomething() const
  {
    std::cout << "Hello, this is a singleton example!\n";
  }

  // Delete copy constructor
  singletonExample(const singletonExample&) = delete;
  singletonExample& operator=(const singletonExample&) = delete;

  // Delete the move constructor
  singletonExample(singletonExample&&) = delete;
  singletonExample& operator=(singletonExample&&) = delete;

  private:
    singletonExample()  {}; // Prevents external creation of object
    ~singletonExample() {};

};

int main()
{
  singletonExample::getInstance().doSomething();

  return 0;
}
