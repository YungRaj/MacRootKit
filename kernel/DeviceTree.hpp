#ifndef __DEVICE_TREE_HPP_
#define __DEVICE_TREE_HPP_

#ifdef __arm64__

namespace xnu
{
    class Kernel;
    class DeviceTree;

    #define DT_KEY_LEN 0x20

    struct DeviceTreeNode
    {
        uint32_t n_properties;
        uint32_t n_children;
    };

    struct DeviceTreeProperty
    {
        char name[DT_KEY_LEN];
        uint32_t size;

        char val[0];
    };

    typedef bool (^dt_node_callback_t) (uint32_t depth, void *node, uint32_t size);
    typedef bool (^dt_property_callback_t) (uint32_t depth, void *prop, uint32_t size);

    static DeviceTree *deviceTree;
    
    PE_state_t* platformExpertState(xnu::Kernel *kernel);
    
    template<typename T>
    T getDeviceTreeHead(xnu::Kernel *kernel)
    {
        PE_state_t *PE = platformExpertState(kernel);

        T deviceTreeHead = reinterpret_cast<T>(PE->deviceTreeHead);
        
        return deviceTreeHead;
    }

    size_t getDeviceTreeSize(xnu::Kernel *kernel)
    {
        PE_state_t *PE = platformExpertState(kernel);

        size_t deviceTreeSize = PE->deviceTreeSize;

        return deviceTreeSize;
    }

    class DeviceTree
    {
        public:
            explicit DeviceTree(xnu::Kernel *kernel) : kernel(kernel), device_tree(xnu::getDeviceTreeHead<mach_vm_address_t>(kernel)), device_tree_sz(xnu::getDeviceTreeSize(kernel)) { }

            template<typename T>
            T getAs() { return reinterpret_cast<T>(device_tree); }
            
            size_t getSize() const { return device_tree_sz; }

            size_t getNodeSize(uint32_t depth, DeviceTreeNode *node);

            DeviceTreeNode* findNode(char *nodename, uint32_t *depth);
            DeviceTreeProperty* findProperty(char *nodename, char *propname);

            bool iterateNode(void **data, void *data_end, uint32_t *depth,
                             dt_node_callback_t node_cb, dt_property_callback_t prop_cb, bool *success);

            bool iterateNodeProperties(void **data, void *data_end, uint32_t *depth,
                                       DeviceTreeNode *node, dt_property_callback_t prop_cb, bool *success);

            void printData(uint8_t *prop_data, uint32_t prop_size);

            void printNode(char *name);

            void print(DeviceTreeNode *node, size_t dt_size);

            template<typename T>
            T dump();

        private:
            xnu::Kernel *kernel;

            mach_vm_address_t device_tree;

            size_t device_tree_sz;
    };

};

#endif

#endif