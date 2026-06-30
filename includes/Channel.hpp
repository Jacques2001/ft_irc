#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

using namespace std;

class Channel
{
    private: 
        string _name;
        string _topic;

        set<int> _members;
        set<int> _operators;
    
    public:
        Channel();
        Channel(const std::string& name);
        ~Channel();

        //getters
        string get_name() const;
        string get_topic() const;
        set<int> get_members() const;

        // members
        void add_member(int fd);
        void remove_member(int fd);
        bool is_member(int fd) const;

        // operators
        void add_operator(int fd);
        void remove_operator(int fd);
        bool is_operator(int fd) const;

        //setter
        void set_topic(string topic);

        // state
        bool empty() const;

        //methods

};

#endif