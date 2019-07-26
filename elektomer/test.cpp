#include <iostream>
    using namespace std;
    int history[720],i;
    int main()
    {


    for(i=0;i<720;i++)
    {
       history[i] = i; //read values to first array
       cout<<history[i]<<",";
    }
    cout<<endl;
    //shift second array to right
    for(i=720;i>=0;i--)
    {
        history[i+1] = history[i]; //move all element to the right except last one
    }
    //output: 1 2 3 4 5 because elements are shifted back by right shift
    cout << "shift: " << endl;
    for(i=0;i<720;i++)
        cout << history[i]<<",";
    cout<<endl;
    return 0;
    
    }