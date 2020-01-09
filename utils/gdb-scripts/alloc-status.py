import gdb
import treelib
from colorama import Fore, Style

class StatusFreeAlloc(gdb.Command):
    """Free List Allocator status: status_free_alloc allocator_obj
    """
    def __init__(self):
        gdb.Command.__init__(self, "status_free_alloc", gdb.COMMAND_DATA, gdb.COMPLETE_SYMBOL, False)

    @staticmethod
    def build_tree(tree, parent, root):
        if parent:
                if parent['left'].address == root.address:
                    position = "Left"
                else:
                    position = "Right"
        else:
                position = "Root"

        if root['color'] == 0x1:
            color = Fore.RED + "RED" + Style.RESET_ALL
        else:
            color = Fore.BLACK + "BLACK" + Style.RESET_ALL

        status = "[%s] Address: 0x%x; BlockSize: %d; Color: %s" % (position, root, root['block_size'], color)
        if not parent:
            current = tree.create_node(status, root)
        else:
            current = tree.create_node(status, root, parent=parent)
            return;

        right_ptr = root['right']
        left_ptr = root['left']

        if right_ptr:
            StatusFreeAlloc.build_tree(tree, root, right_ptr)

        if left_ptr:
            StatusFreeAlloc.build_tree(tree, root, left_ptr)


    def invoke(self, arg, from_tty):
        if not arg:
            print("usage: status_free_alloc object")
            return
        allocator = gdb.parse_and_eval(arg)
        print("Managed block address: 0x%x" % allocator['mem_block'])
        node_ptr = gdb.lookup_type('struct node').pointer()
        if not allocator['root_node']:
            print("The tree does not contain any nodes")
            return
        root_node = allocator['root_node'].cast(node_ptr)


        tree = treelib.Tree()
        StatusFreeAlloc.build_tree(tree, None, root_node)
        tree.show()
        


StatusFreeAlloc()
