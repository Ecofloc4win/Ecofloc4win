#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <list>

class process
{
    private:

        /**
        * @brief the pid of the process
        */
        string m_pid;

        /**
        * @brief the name of the process
        */
        string m_name;

    public:

        /**
        * @brief Constructor for the process class.
        * 
        * @param pid The process ID.
        * @param name The process name.
        */
        process(string pid, string name);

        /**
        * @brief Getter for the process ID.
        * 
        * @return string The process ID.
        */
        string getPid() const;

        /**
        * @brief Getter for the process name.
        * 
        * @return string The process name.
        */
        string getName();

        /**
        * @brief Setter for the process ID.
        *
        * @param pid The process ID.
        */
        void setPid(string pid);

        /**
        * @brief Setter for the process name.
        *
        * @param name The process name.
        */ 
        void setName(string name);
};

