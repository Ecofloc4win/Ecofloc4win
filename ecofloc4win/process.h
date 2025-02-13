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
        std::string m_pid;

        /**
        * @brief the name of the process
        */
        std::string m_name;

    public:

        /**
        * @brief Constructor for the process class.
        * 
        * @param pid The process ID.
        * @param name The process name.
        */
        process(std::string pid, std::string name);

        /**
        * @brief Getter for the process ID.
        * 
        * @return std::string The process ID.
        */
        std::string getPid() const;

        /**
        * @brief Getter for the process name.
        * 
        * @return std::string The process name.
        */
        std::string getName();

        /**
        * @brief Setter for the process ID.
        *
        * @param pid The process ID.
        */
        void setPid(std::string pid);

        /**
        * @brief Setter for the process name.
        *
        * @param name The process name.
        */ 
        void setName(std::string name);
};

