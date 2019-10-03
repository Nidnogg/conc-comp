import glob
import json
import os
import subprocess
import sys

import pydot
import re
from pathlib import Path
from typing import Dict, List, Set, Tuple
from enum import Enum

## AUXILIARY CLASSES ##
class EnumEncoder(json.JSONEncoder):
    def default(self, obj):
        try:
            return json.JSONEncoder.default(self, obj)
        except:
            return {"__enum__": str(obj)}

class JsonRepr:
    def __repr__(self) -> str:
        return json.dumps(self.__dict__, cls=EnumEncoder)

    def __str__(self) -> str:
        return json.dumps(self.__dict__, cls=EnumEncoder)

class SummaryState(Enum):
    """
    A `enum` that specifies the Summary's current progress
    """
    NOT_PARSED = 0,
    PROCESSING = 1,
    SEARCHING = 2,
    DONE = 3


class NodeType(Enum):
    """
    A `enum` that specifies if the node is inside a loop and should, therefore, assume that all that happens inside it
    happens more than once
    """
    COMMON = 0,
    LOOP = 1


class VariableStats(JsonRepr):
    """
    Used to keep track of a certain variable, and the important information that is needed to test for a race condition
    """

    def __init__(self, writen: bool = False, locks: Dict[str, int] = None):
        self.writen: bool = writen
        self.locks: Dict[str, int] = locks if (locks is not None) else {}


def protected_locks(locks: Dict[str, int]) -> Set[str]:
    s = set()
    for k, v in locks.items():
        if v > 0:
            s.add(k)
    return s


def merge_sequential_locks(consider1: bool, locks1: Dict[str, int], consider2: bool, locks2: Dict[str, int]) -> Dict:
    """

    :param consider1: Whether any variable uses `locks1`
    :param locks1: The lock's state for a variable in a certain context
    :param consider2: Whether any variable uses `locks2`
    :param locks2: The lock's state for a variable in a certain context
    :return: The merged sequential context
    """
    if consider1 and consider2:
        return {k: min(i for i in (locks1.get(k,0), locks2.get(k,0)) if i is not None) for k in locks1.keys() | locks2.keys()}
    elif consider1:
        return locks1.copy()
    elif consider2:
        return locks2.copy()
    else:
        return {}


def merge_concurrent_locks(locks1_activated: bool, locks1: Set[str], locks2_activated: bool, locks2: Set[str]) -> Set:
    if locks1_activated and locks2_activated:
        return locks1 & locks2
    elif locks1_activated:
        return locks1.copy()
    elif locks2_activated:
        return locks2.copy()
    else:
        return set()


class RaceVariableSummary(JsonRepr):
    """
    Used to keep track of a certain race condition, and if new locks (opened before the node where it was created) may
    block it
    """
    racing: bool = False
    other_racing: bool = False

    read: bool = False
    current_thread_read_locks: Dict[str, int] = {}
    written: bool = False
    current_thread_write_locks: Dict[str, int] = {}

    other_read: bool = False
    other_thread_read_locks: Set[str] = set()
    other_written: bool = False
    other_thread_write_locks: Set[str] = set()

    def always_racing(self):
        """
        Called to avoid reduntant methods in case of certainty of a race condition (only happens if a different thread
        started in this one has race condition with another thread)
        """
        self.racing = True
        self.other_racing = True

        self.merge_sequential_nodes = lambda next_node, open_locks: None
        self.merge_exclusive_nodes = lambda other: None
        self.add_sequential_access = lambda locks, written: None
        self.add_concurrent_access = lambda other: None

    def merge_sequential_nodes(self, next_node, open_locks: Dict[str, int]):
        """
        Merges the race summary for sequential nodes into the current summary

        :param next_node: The second RaceVariableSummary
        :param open_locks: State of the locks at the end of the node
        """
        if next_node.other_racing or \
                (self.other_written and next_node.other_read and
                 len(self.other_thread_write_locks & next_node.other_thread_read_locks) == 0) or \
                (self.other_read and next_node.other_written and
                 len(self.other_thread_read_locks & next_node.other_thread_write_locks) == 0):
            self.always_racing()
            return

        if next_node.read:
            next_node_updated_read_locks = next_node.current_thread_read_locks.copy()
            next_node_updated_write_locks = next_node.current_thread_write_locks.copy()
            for k, v in open_locks.items():
                next_node_updated_read_locks[k] = next_node_updated_read_locks.get(k, 0) + v
                next_node_updated_write_locks[k] = next_node_updated_write_locks.get(k, 0) + v

            if not self.racing and next_node.racing:
                if next_node.written:
                    self.racing = self.racing or len(protected_locks(next_node_updated_write_locks) &
                                                     next_node.other_thread_read_locks) == 0
                if next_node.read and next_node.other_written:
                    self.racing = self.racing or len(protected_locks(next_node_updated_read_locks) &
                                                     next_node.other_thread_write_locks) == 0

            if not self.racing and (self.other_read and next_node.written):
                self.racing = len(self.other_thread_read_locks & protected_locks(next_node_updated_write_locks)) == 0

            if not self.racing and (self.other_written and next_node.read):
                self.racing = len(self.other_thread_write_locks & protected_locks(next_node_updated_read_locks)) == 0

            # Atualizar locks nos acessos sequenciais
            self.current_thread_read_locks = merge_sequential_locks(self.read, self.current_thread_read_locks,
                                                                    next_node.read, next_node_updated_read_locks)
            self.current_thread_write_locks = merge_sequential_locks(self.written, self.current_thread_write_locks,
                                                                     next_node.written, next_node_updated_write_locks)
            self.read = self.read or next_node.read
            self.written = self.written or next_node.written

        # Atualizar locks nos acessos concorrentes
        self.other_thread_read_locks = merge_concurrent_locks(self.other_read, self.other_thread_read_locks,
                                                              next_node.other_read, next_node.other_thread_read_locks)
        self.other_thread_write_locks = merge_concurrent_locks(self.other_written, self.other_thread_write_locks,
                                                               next_node.other_written,
                                                               next_node.other_thread_write_locks)
        self.other_read = self.other_read or next_node.other_read
        self.other_written = self.other_written or next_node.other_written

    def merge_exclusive_nodes(self, other):
        """
        Merges the race summary for two nodes that cant happen simultaniously (if-else)

        :param other: The other RaceVariableSummary
        """
        if other.other_racing:
            self.always_racing()
            return
        self.racing = self.racing or other.racing

        self.current_thread_read_locks = merge_sequential_locks(self.read, self.current_thread_read_locks,
                                                                other.read, other.current_thread_read_locks)
        self.current_thread_write_locks = merge_sequential_locks(self.written, self.current_thread_write_locks,
                                                                 other.written, other.current_thread_write_locks)
        self.read = self.read or other.read
        self.written = self.written or other.written

        # Atualizar locks nos acessos concorrentes
        self.other_thread_read_locks = merge_concurrent_locks(self.other_read, self.other_thread_read_locks,
                                                              other.other_read, other.other_thread_read_locks)
        self.other_thread_write_locks = merge_concurrent_locks(self.other_written, self.other_thread_write_locks,
                                                               other.other_written, other.other_thread_write_locks)
        self.other_read = self.other_read or other.other_read
        self.other_written = self.other_written or other.other_written

    def add_concurrent_access(self, concurrent_summary):
        """
        Indicates that a new thread has accessed the variable and merges the summary into the current one. Updates the
        locks in common and checks race against previous concurrent threads

        :param concurrent_summary: The summary for the variable accessed in another thread
        """
        if concurrent_summary is None:
            return
        if concurrent_summary.racing:
            self.always_racing()
            return

        new_read_locks = merge_concurrent_locks(concurrent_summary.read,
                                                protected_locks(concurrent_summary.current_thread_read_locks),
                                                concurrent_summary.other_read,
                                                concurrent_summary.other_thread_read_locks)
        if self.other_written and len(self.other_thread_write_locks & new_read_locks) == 0:
            self.always_racing()

        new_write = concurrent_summary.written or concurrent_summary.other_written
        if new_write:
            new_write_locks = merge_concurrent_locks(concurrent_summary.written,
                                                     protected_locks(concurrent_summary.current_thread_write_locks),
                                                     concurrent_summary.other_written,
                                                     concurrent_summary.other_thread_write_locks)
            if self.other_read and len(self.other_thread_read_locks & new_write_locks) == 0:
                self.always_racing()
        else:
            new_write_locks = set()

        self.other_thread_read_locks = merge_concurrent_locks(self.other_read, self.other_thread_read_locks,
                                                              True, new_read_locks)
        self.other_thread_write_locks = merge_concurrent_locks(self.other_written, self.other_thread_write_locks,
                                                               new_write, new_write_locks)

        self.other_read = True
        self.other_written = self.other_written or new_write

    def add_sequential_access(self, locks: Dict[str, int], written: bool):
        """
        Indicates a new access to the variable in the current thread

        :param locks: The locks currently locked during this access
        :param written: Tells if this access is a read or write access
        """

        if not self.racing and (self.other_written or (written and self.other_read)):
            other_locks: Set[str] = (self.other_thread_read_locks if written else self.other_thread_write_locks)
            self.racing = len(other_locks & protected_locks(locks)) == 0

        self.current_thread_read_locks = merge_sequential_locks(self.read, self.current_thread_read_locks, True, locks)
        self.current_thread_write_locks = merge_sequential_locks(self.written, self.current_thread_write_locks, written,
                                                                 locks)

        self.read = True
        self.written = self.written or written

    def loop(self):
        """
        Indicates that the current node is part of a loop.

        Tip: Call this method on the first node of a loop after doing the Depth-First Search
        """
        if self.other_written and len(self.other_thread_read_locks) == 0:
            self.always_racing()
            return
        if not self.racing and self.written and self.other_read:
            self.racing = len(self.other_thread_read_locks & protected_locks(self.current_thread_write_locks)) == 0
        if not self.racing and self.other_written and self.read:
            self.racing = len(self.other_thread_write_locks & protected_locks(self.current_thread_read_locks)) == 0


class Summary(JsonRepr):
    """
    Generic sumary, used for all functions
    """

    class SummaryWarning(Enum):
        """
        A `enum` that specifies if the node is inside a loop and should, therefore, assume that all that happens inside it
        happens more than once
        """
        INCONSISTENT_LOCKS = 0

    def __init__(self):
        self.status: SummaryState = SummaryState.NOT_PARSED
        self.type: NodeType = NodeType.COMMON
        self.variables: Dict[str, RaceVariableSummary] = {}  # Global variables that may have race conditions
        self.open_locks: Dict[str, int] = {}
        self.warnings: Set[Summary.SummaryWarning] = set()


class NodeSummary(Summary):
    """
    Summary specific to nodes, containing some additional information
    """

    def __init__(self) -> None:
        super().__init__()
        self.out_nodes: Set[str] = set()


def variable_has_race(v1: VariableStats, v2: VariableStats) -> bool:
    """
    Checks if both variable access (assuming they come from two threads running concurrently) can have a race condition

    :param v1: A valid variable stat
    :param v2: A valid variable stat
    :return: True if race condition is possible
    """
    is_locked = False
    for k, v in v1.locks.items():
        if k in v2.locks and v > 0 and v2.locks[k] > 0:
            is_locked = True
            break
    return (v1.writen or v2.writen) and not is_locked


def deep_search_node(node_summaries: Dict[str, NodeSummary], node_name: str) -> None:
    """
    Performs a Depth-First Search across the nodes for a given function and merges them

    :param node_summaries:
    :param node_name: The name of the next node to search
    """
    root = node_summaries[node_name]
    if root.status != SummaryState.PROCESSING:
        return
    root.status = SummaryState.SEARCHING
    out_names = [x for x in root.out_nodes if node_summaries[x].status != SummaryState.NOT_PARSED]

    if len(out_names) > 0:
        for out in out_names:
            root.out_nodes.remove(out)
            out_s = node_summaries[out]
            if out_s.status == SummaryState.PROCESSING:
                deep_search_node(node_summaries, out)
        merge_summary_in_node(root, *[node_summaries[x] for x in out_names])

    if len(root.out_nodes) == 0:
        root.status = SummaryState.DONE
    else:
        root.status = SummaryState.PROCESSING


def merge_summary_in_node(parent: NodeSummary, *children: Summary):
    """
    Merges the summaries for a node and a function/node, taking the necessary precautions if they are sequential

    :param parent: The parent node summary
    :param children: The summary for each child
    """
    if len(children) == 0:
        return  # Nothing to merge
    else:
        child = children[0]
        if len(children) > 1:
            # Merge all branches in a single node
            for c in children[1:]:
                for v, r in c.variables.items():
                    if v not in child.variables:
                        child.variables[v] = RaceVariableSummary()
                    child.variables[v].merge_exclusive_nodes(r)
                for l, i in c.open_locks.items():
                    if l not in child.open_locks:
                        child.open_locks[l] = 0
                    if child.open_locks[l] != i:
                        child.open_locks[l] = min(child.open_locks[l], i)
                        child.warnings.add(Summary.SummaryWarning.INCONSISTENT_LOCKS)
                child.warnings = child.warnings.union(c.warnings)

    for v, r in child.variables.items():
        if v not in parent.variables:
            parent.variables[v] = RaceVariableSummary()
        parent.variables[v].merge_sequential_nodes(r, parent.open_locks)
    for l, i in child.open_locks.items():
        parent.open_locks[l] = parent.open_locks.get(l, 0) + i
    parent.warnings = parent.warnings.union(child.warnings)


def merge_variable(dest: VariableStats, merge: VariableStats):
    """
    Given two access to the same variable that happen in the same context (in a different thread, even if not the same
    one for both, or in the current thread), merge their info so as to keep just the essential information.

    :param dest: The variable where the merged info for `dest` and `merge` will be stored
    :param merge: The variable info to be merged with `dest`
    """

    # If the variable is written even once that's all that matters
    dest.writen = dest.writen or merge.writen

    # Only the locks shared by both access truly protect the variable
    k1 = set(dest.locks.keys())
    k2 = set(merge.locks.keys())
    for k in k1 - k2:
        del dest.locks[k]
    for k, v in dest.locks.items():
        dest.locks[k] = min(v, merge.locks[k])


def include_variable_access_in_node(s: NodeSummary, v: str, written: bool):
    if v not in s.variables:
        s.variables[v] = RaceVariableSummary()
    s.variables[v].add_sequential_access(s.open_locks, written)


def parse_line(node_summaries: Dict[str, NodeSummary], function_summaries: Dict[str, Summary], g: pydot.Dot, s: NodeSummary, lines: List[str], i: int):
    try:
        line: str = lines[i].split(':', maxsplit=1)[1]
        ss: List[str] = line.split('=', maxsplit=1)
        if len(ss) == 2:
            v = re.match(r"\[`(.*)'\]\\l\\", ss[1])
            if v is not None:
                # Global/Static variable is read
                k = v.groups()[0]
                include_variable_access_in_node(s, k, False)
            else:
                v = re.match(r"\\ \[`(.*)'\]", ss[0])
                if v is not None:
                    # Global/Static variable is written
                    k = v.groups()[0]
                    include_variable_access_in_node(s, k, True)
                else:
                    v = re.match(r"call\\ \[`(.*)'\].*", ss[1])
                    if v is not None:
                        # Function is called
                        f = v.groups()[0]
                        if f == "pthread_create":
                            t = re.search(r"`(.*)'", lines[i - 3]).groups()[0]
                            crawl_function(node_summaries, function_summaries, t, g.get_parent_graph())
                            ts = function_summaries[t]

                            for k, v in ts.variables.items():
                                if k not in s.variables:
                                    s.variables[k] = RaceVariableSummary()
                                s.variables[k].add_concurrent_access(v)
                        elif f == "pthread_mutex_lock":
                            lock = re.search(r"`(.*)'", lines[i - 1]).groups()[0]
                            # TODO check anonymous lock (pointer or somthing of the sort)
                            s.open_locks[lock] = s.open_locks.get(lock, 0) + 1
                        elif f == "pthread_mutex_unlock":
                            lock = re.search(r"`(.*)'", lines[i - 1]).groups()[0]
                            s.open_locks[lock] = s.open_locks.get(lock, 0) - 1
                        elif f == "pthread_mutex_init":
                            pass  # TODO
                        elif f == "pthread_mutex_destroy":
                            pass  # TODO
                        elif f == "pthread_join":
                            pass  # TODO
                        else:
                            crawl_function(node_summaries, function_summaries, f, g.get_parent_graph())
                            # Merge function into node
                            fs = function_summaries[f]
                            merge_summary_in_node(s, fs)
        else:
            v = re.match(r"\\ call\\ \[`(.*)'\]\\ argc:0\\l\\", ss[0])
            if v is not None:
                # Function is called
                f = v.groups()[0]
                crawl_function(node_summaries, function_summaries, f, g.get_parent_graph())
                # Merge function into node
                fs = function_summaries[f]
                merge_summary_in_node(s, fs)
    except IndexError:
        pass


def crawl_subgraph(node_summaries: Dict[str, NodeSummary], function_summaries: Dict[str, Summary], g: pydot.Dot) -> str:
    """
    Creates the `NodeSummary` for all nodes in this subgraph and stores them in `node_summaries`

    :param node_summaries:
    :param function_summaries:
    :param g: The compiler-generated subgraph
    """
    nodes_list = g.get_nodes()

    for loop in g.get_subgraphs():
        node_name: str = crawl_subgraph(node_summaries, function_summaries, loop)
        node_summaries[node_name].type = NodeType.LOOP
        for k, v in node_summaries[node_name].variables.items():
            v.loop()

    for node in nodes_list:
        s = node_summaries[node.get_name()]
        s.status = SummaryState.PROCESSING

        lines: List[str] = node.get_attributes()['label'].split('\n')
        for i in range(1, len(lines) - 1):  # Ignore the first and last lines wich I know don't have any calls
            parse_line(node_summaries, function_summaries, g, s, lines, i)

    first_node: str = nodes_list[0].get_name()
    deep_search_node(node_summaries, first_node)
    return first_node


def crawl_function(node_summaries: Dict[str, NodeSummary], function_summaries: Dict[str, Summary], func_name: str, g: pydot.Dot):
    """
    Creates the `Summary` for the function `func_name` and stores it in the global variable `function_summaries`

    :param node_summaries:
    :param function_summaries:
    :param func_name: The name of the function whose Summary will be created
    :param g: The compiler-generated graph
    """
    if func_name in function_summaries:
        if function_summaries[func_name].status == SummaryState.PROCESSING:
            function_summaries[func_name].type = NodeType.LOOP
        return

    s = function_summaries[func_name] = Summary()

    func_graph_list = g.get_subgraph("\"cluster_" + func_name + "\"")
    if len(func_graph_list) < 1:  # Função pertence a alguma biblioteca, logo não é motivo de preocupação
        s.status = SummaryState.DONE
        return
    func_graph = func_graph_list[0]

    s.status = SummaryState.PROCESSING

    for edge in func_graph.get_edges():
        src: str = edge.get_source().split(':', maxsplit=1)[0]
        dst: str = edge.get_destination().split(':', maxsplit=1)[0]
        if src not in node_summaries:
            node_summaries[src] = NodeSummary()
        if dst not in node_summaries:
            node_summaries[dst] = NodeSummary()
        node_summaries[src].out_nodes.add(dst)

    first_node = crawl_subgraph(node_summaries, function_summaries, func_graph)

    s.variables = node_summaries[first_node].variables
    s.open_locks = node_summaries[first_node].open_locks

    if s.type == NodeType.LOOP:
        for v, r in s.variables.items():
            r.loop()

    s.status = SummaryState.DONE
    return

def main(path: str, verbose: bool = False) -> (int, Dict[str, Summary]):
    def clean():
        """
        Deletes all files in the current folder that weren't there when the script started
        """
        for f in os.listdir(os.getcwd()):
            if f.startswith(file.stem + ".") and f not in existing_files:
                if os.path.isfile(f):
                    os.remove(f)
                elif os.path.isdir(f):
                    os.rmdir(f)

    verbose and print("Bem vindo ao detector de condição de corrida para programas em C")

    # path = "pthread.c"  # TODO remover após testes
    if path is None:
        path = input("Defina o nome do arquivo a ser verificado: ")

    file = Path(path)
    if not file.is_file():
        print("Arquivo solicitado não existe")
        exit(1)

    filename = file.name
    verbose and print("'" + filename + "' será analizado.")

    existing_files = [f for f in os.listdir(os.getcwd()) if str(f).startswith(file.stem)]

    verbose and print("Gerando arquivos auxiliares...")
    cmd = ["gcc", "-fdump-rtl-expand-graph", "-c", path]
    p = subprocess.Popen(cmd)
    p.wait()
    if p.returncode != 0:
        print("Compilação não sucedida")
        clean()
        return 1

    verbose and print("Identificando funções chamadas em thread...")

    function_summaries: Dict[str, Summary] = {}
    node_summaries: Dict[str, NodeSummary] = {}

    graph = pydot.graph_from_dot_file(glob.glob(filename + ".*.expand.dot")[0])[0]
    crawl_function(node_summaries, function_summaries, "main", graph)

    verbose and print("Analisando possíveis corridas...")
    verbose and print([k for k, v in function_summaries["main"].variables.items() if v.racing])
    clean()
    return 0, function_summaries


if __name__ == '__main__':
    exit(main(sys.argv[1] if 1 < len(sys.argv) else None, True)[0])
