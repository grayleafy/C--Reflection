#include<cstdio>
#include"Object.h"
#include"Person.h"

using namespace std;

int main(){
    Type &personType = Type::getTypeByName("Person");
    Object &p = *(personType.createInstance());

    cout << "age:" << p.getType().getField(&p, "age").cast<int>() << endl;
    cout << "name:" << p.getType().getField(&p, "name").cast<string>() << endl;

    Type &type = p.getType();
    Method &method = type.getMethod("addAge");
    method.invoke<void, int>(&p, 10);


    Person p2 = p.cast<Person>();
    cout << "age:" << p2.age << endl;
    cout << "name:" << p2.name << endl;

    
    {
        Object a = 100;
        Object b = 'b';

        Object c = std::move(b);

        
        cout << "a: type = " << a.getType().name << ", val = " << a.cast<int>() << endl;
        cout << "c: type = " << c.getType().name << ", val = " << c.cast<char>() << endl;

    }
}