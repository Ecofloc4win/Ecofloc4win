/**
 * @file process.h
 * @brief Implementation of the process class
 * @author Ecofloc's Team
 * @date 2025-02-03
 */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <list>

/**
 * @class process
 * @brief Class to manage process operations.
 */
class process
{
    private:

        /**
        * @var {std::string} m_pid
        * @brief the pid of the process
        */
        std::string m_pid;

        /**
        * @var {std::string} m_name
        * @brief the name of the process
        */
        std::string m_name;

    public:

        /**
        * @brief Constructor for the process class.
        * 
        * @param {std::string} pid The process ID.
        * @param {std::string} name The process name.
        */
        process(std::string pid, std::string name);

        /**
        * @brief Getter for the process ID.
        * @function getPid
        * @returns {std::string} The process ID.
        */
        std::string getPid() const;

        /**
        * @brief Getter for the process name.
        * @function getName
        * @return {std::string} The process name.
        */
        std::string getName();

        /**
        * @brief Setter for the process ID.
        * @function setPid
        * @param {std::string} pid The process ID.
        */
        void setPid(std::string pid);

        /**
        * @brief Setter for the process name.
        * @function setName
        * @param {std::string} name The process name.
        */ 
        void setName(std::string name);
};

