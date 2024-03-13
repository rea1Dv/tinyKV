#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <vector>
#include <memory>

#define STORE_PATH "dumpFile"

namespace node {

template<typename K,typename V>
class Node{
public:
    Node(){}
    ~Node(){}

    Node(const K k,const V v,int level)
    : key_(k),value_(v),node_level_(level),
      forward_(level,nullptr){}
    K GetKey() const {return key_;}
    V GetValue() const {return value_;}
    void GetValue(V value){value_=value;}

    // use to store next Node for each level
    std::vector<std::shared_ptr<Node<K,V>>> forward_;
    //level of specific Node
    int node_level_;

private:
    K key_;
    V value_;
};

template<typename K,typename V>
using NodeVec=std::vector<std::shared_ptr<node::Node<K,V>>>;
}

namespace skip_list {

template<typename K,typename V>
class SkipList{
public:
    SkipList(int);
    ~SkipList();
    int GetRandomLevel() const;
    std::shared_ptr<node::Node<K,V>> CreateNode(K,V,int);
    int InsertElement(K,V);
    void PrintList() const;
    bool SearchElement(K) const;
    void DeleteElement(K);
    void DumpFile();
    void LoadFile();
    int Size() const {return elementCount_;}

private:
    void GetKeyValueFromString(const std::string& str,std::string& key,std::string& value) const;
    bool IsValidString(const std::string& str) const;

private:
    int maxLevel_;
    int currentLevel_;
    std::shared_ptr<node::Node<K,V>> header_;
    std::ofstream fileWriter_;
    std::ifstream fileReader_;
    int elementCount_;
    std::mutex mutex_;
};

template<typename K,typename V>
SkipList<K,V>::SkipList(int maxLevel)
    : maxLevel_(maxLevel),
      currentLevel_(0),elementCount_(0){
    header_=std::make_shared<node::Node<K,V>>(K(),V(),maxLevel);
}

template<typename K,typename V>
SkipList<K,V>::~SkipList(){
    if(fileWriter_.is_open()){
        fileWriter_.close();
    }
    if(fileReader_.is_open()){
        fileReader_.close();
    }
}

template<typename K,typename V>
int SkipList<K,V>::GetRandomLevel() const{
    int level=1;
    while(rand()%2){
        level++;
    }
    level=(level<maxLevel_)?level:maxLevel_;
    return level;
}

template<typename K,typename V>
std::shared_ptr<node::Node<K,V>> SkipList<K,V>::CreateNode(const K k,V v,int level){
    auto node=std::make_shared<node::Node<K,V>>(k,v,level);
    return node;
}

template<typename K,typename V>
int SkipList<K,V>::InsertElement(const K key,const V value){
    std::unique_lock<std::mutex>  lock(mutex_);
    auto current=header_;

    auto update=node::NodeVec<K,V>(maxLevel_+1);

    for(int i=currentLevel_;i>=0;i--){
        while(current->forward_[i]!=nullptr&&current->forward_[i]->GetKey()<key){
            current=current->forward_[i];
        }
        update[i]=current;
    }

    current=current->forward_[0];
    
    if(current!=nullptr&&current->GetKey()==key){
        std::cout<<"key: "<<key<<" exists"<<std::endl;
        lock.unlock();
        return 1;
    }

    if(current==nullptr||current->GetKey()!=key){
        int random_level=GetRandomLevel();

        if(random_level>currentLevel_){
            for(int i=currentLevel_+1;i<random_level+1;i++){
                update[i]=header_;
            }
            currentLevel_=random_level;
        }

        auto insertedNode=CreateNode(key,value,random_level);

        for(int i=0;i<random_level;i++){
            insertedNode->forward_[i]=update[i]->forward_[i];
            update[i]->forward_[i]=insertedNode;
        }
        std::cout<<"Successfully inserted key:"<<key<<", value:"<<value<<std::endl;
        elementCount_++;
    }
    lock.unlock();
    return 0;
}

template<typename K, typename V>
void SkipList<K,V>::PrintList()const{
    std::cout<<"\n*****Skip List*****"<<"\n";
    for(int i=0;i<=currentLevel_;i++){
        auto node=header_->forward_[i];
        std::cout<<"Level: "<<i;
        while(node!=nullptr){
            std::cout<<node->GetKey()<<":"<<node->GetValue()<<";  ";
            node=node->forward_[i];
        }
        std::cout<<std::endl;
    }
}

template<typename K, typename V>
void SkipList<K,V>::DumpFile(){
    std::cout << "dump_file-----------------" << std::endl;
    fileWriter_.open(STORE_PATH);
    auto node=header_->forward_[0];
    while(node!=nullptr){
        fileWriter_<< node->GetKey() << ":" << node->GetValue() << "\n";
        std::cout << node->GetKey() << ":" << node->GetValue() << ";\n";
        node=node->forward_[0];
    }
    fileWriter_.flush();
    fileWriter_.close();
    return;
}
template<typename K, typename V>
void SkipList<K,V>::LoadFile(){
    fileReader_.open(STORE_PATH);
    std::cout << "load_file-----------------" << std::endl;
    std::string str;
    std::string key;
    std::string value;
    while(getline(fileReader_,str)){
        GetKeyValueFromString(str,key,value);
        if(key.empty()||value.empty()){
            continue;
        }
        InsertElement(key,value);
        std::cout << "key:" << key << "value:" << value << std::endl;
    }
    fileReader_.close();
}
template<typename K, typename V>
void SkipList<K,V>::GetKeyValueFromString(
    const std::string& str,std::string& key,std::string& value)const{
    if(!IsValidString(str)){
        return;
    }
    key=str.substr(0,str.find(":"));
    value=str.substr(str.find(":")+1,str.length());
}
template<typename K, typename V>
void SkipList<K,V>::DeleteElement(K key){
    std::unique_lock<std::mutex>  lock(mutex_);
    auto current=header_;

    auto update=node::NodeVec<K,V>(maxLevel_+1);

    for(int i=currentLevel_;i>=0;i--){
        while(current->forward_[i]!=nullptr&&current->forward_[i]->GetKey()<key){
            current=current->forward_[i];
        }
        update[i]=current;
    }

    current=current->forward_[0];
    if(current!=nullptr&&current->GetKey()==key){
        for(int i=0;i<=currentLevel_;i++){
            if(update[i]->forward_[i]!=current) break;
            update[i]->forward_[i]=current->forward_[i];
        }
    
        while(currentLevel_>0&&header_->forward_[currentLevel_]==nullptr){
            currentLevel_--;
        }
        std::cout << "Successfully deleted key "<< key << std::endl;
        elementCount_--;
    }
    lock.unlock();
    return;
}
template<typename K, typename V>
bool SkipList<K,V>::SearchElement(K key)const{
    std::cout << "search_element-----------------" << std::endl;

    auto current=header_;
    for(int i=currentLevel_;i>=0;i--){
        while(current->forward_[i]!=nullptr&&current->forward_[i]->GetKey()<key){
            current=current->forward_[i];
        }
    }
    current = current->forward_[0];
  
    if (current && current->GetKey() == key) {
        std::cout << "Found key: " << key << ", value: " << current->GetValue() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}
}