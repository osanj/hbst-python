import pytest

import hbst
import numpy as np


def build_descriptors(n: int, desc_size: int) -> np.ndarray:
    descs = np.zeros((n, desc_size), dtype=np.uint8)
    for i in range(n):
        descs[i] = i
    return descs


@pytest.mark.parametrize("tree_cls", [hbst.BinaryTree128])  #, hbst.BinaryTree256, hbst.BinaryTree512])
def test_basic_tree_usage(tree_cls):
    tree = tree_cls()
    desc_size = tree.get_desc_size_in_bytes()

    n1 = 3
    n2 = 5
    train_desc = build_descriptors(n1 + n2, desc_size)
    train_idx = np.arange(len(train_desc), dtype=np.uint64)
    s1 = slice(0, n1)
    s2 = slice(n1, n1 + n2)

    tree.add(0, train_idx[s1], train_desc[s1], hbst.SplittingStrategy.DoNothing, False)
    tree.add(1, train_idx[s2], train_desc[s2], hbst.SplittingStrategy.DoNothing, False)

    tree.train(hbst.SplittingStrategy.SplitEven)
    assert tree.size() == 2
