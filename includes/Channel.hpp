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
        set<int> _invited;

        bool _invite_only;
        bool _topic_restricted;

        bool _has_key;
        string _key;

        bool _has_limit;
        int _limit;
    
    public:
        Channel();
        Channel(const std::string& name);
        ~Channel();

        //getters
        string get_name() const;
        string get_topic() const;
        string get_key() const;
        set<int> get_members() const;
        int get_limit() const;
        bool is_invite_only() const;
        bool is_topic_restricted() const;
        bool has_key() const;
        bool has_limit() const;

        // members
        void add_member(int fd);
        void remove_member(int fd);
        bool is_member(int fd) const;
        void add_invited(int fd);
        void remove_invited(int fd);
        bool is_invited(int fd) const;

        // operators
        void add_operator(int fd);
        void remove_operator(int fd);
        bool is_operator(int fd) const;

        //setter
        void set_topic(string topic);
        void set_invite_only(bool value);
        void set_topic_restricted(bool value);
        void set_key(string key);
        void set_limit(int limit);
        void remove_key();
        void remove_limit();

        // state
        bool empty() const;

        //methods





};

#endif