/*
 * ResourceManager.h
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#ifndef USER_APP_PAGES_INC_RESOURCEMANAGER_H_
#define USER_APP_PAGES_INC_RESOURCEMANAGER_H_

#include <vector>

class ResourceManager
{

public:
    ResourceManager();
    ~ResourceManager();

    bool AddResource(const char* name, void* ptr);
    bool RemoveResource(const char* name);
    void* GetResource(const char* name);
    void SetDefault(void* ptr);

private:
    typedef struct ResourceNode
    {
        const char* name;
        void* ptr;

        bool operator==(const struct ResourceNode n) const
        {
            return (this->name == n.name && this->ptr == n.ptr);
        }
    } ResourceNode_t;

private:
    std::vector<ResourceNode_t> NodePool;
    void* DefaultPtr;
    bool SearchNode(const char* name, ResourceNode_t* node);
};



#endif /* USER_APP_PAGES_INC_RESOURCEMANAGER_H_ */
