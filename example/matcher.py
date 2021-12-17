from typing import Dict, List, Mapping, Optional, Tuple

import cv2 as cv
import hbst
import numpy as np


class Matcher(object):

    def __init__(self, features: Dict[int, Tuple[List[cv.KeyPoint], np.ndarray]]) -> None:
        self.features = features

    def get_keypoint(self, im_id: int, desc_id: int) -> cv.KeyPoint:
        return self.features[im_id][0][desc_id]

    def get_descriptor(self, im_id: int, desc_id: int) -> np.ndarray:
        return self.features[im_id][1][desc_id]

    def match(self, query_descs: np.ndarray) -> Dict[int, Tuple[List[cv.DMatch], Mapping[int, cv.KeyPoint]]]:
        raise NotImplementedError()


class BruteForceMatcher(Matcher):

    def __init__(self, norm: int, features: Dict[int, Tuple[List[cv.KeyPoint], np.ndarray]],
                 match_ratio: Optional[float] = 0.7) -> None:
        super().__init__(features)
        self.bfm = cv.BFMatcher(norm)
        self.match_ratio = match_ratio

    def match(self, query_descs: np.ndarray) -> Dict[int, Tuple[List[cv.DMatch], Mapping[int, cv.KeyPoint]]]:
        matches = {}
        for key in self.features.keys():
            matches[key] = self.match_single(key, query_descs)
        return matches

    def match_single(self, im_id: int, query_descs: np.ndarray) -> Tuple[List[cv.DMatch], Mapping[int, cv.KeyPoint]]:
        kp, descs = self.features[im_id]
        matches = self.bfm.knnMatch(query_descs, descs, k=2)
        good_matches: List[cv.DMatch] = []
        for m, n in matches:
            if self.match_ratio is None or m.distance < self.match_ratio * n.distance:
                good_matches.append(m)
        return good_matches, {k: v for k, v in enumerate(kp)}


class HbstMatcher(Matcher):

    def __init__(self, bits: int, features: Dict[int, Tuple[List[cv.KeyPoint], np.ndarray]],
                 max_dist: int = 25, pad_if_required: bool = False) -> None:
        super().__init__(features)
        tree_cls = {128: hbst.BinaryTree128,
                    256: hbst.BinaryTree256,
                    488: hbst.BinaryTree488,
                    512: hbst.BinaryTree512}.get(bits)
        if tree_cls is None:
            raise ValueError(f"No tree available with descriptor size of {bits} bits")

        self.tree = tree_cls(pad_if_required)
        self.max_dist = max_dist
        for im_id, (_, descs) in enumerate(features.items()):
            self.tree.add(im_id, np.arange(len(descs), dtype=np.uint64), descs)
        self.tree.train(hbst.Splittingintategy.SplitEven)

    def match(self, query_descs: np.ndarray) -> Dict[int, Tuple[List[cv.DMatch], Mapping[int, cv.KeyPoint]]]:
        all_matches = self.tree.match(np.arange(len(query_descs), dtype=np.uint64), query_descs,
                                      max_distance=self.max_dist)
        all_matches_by_im_id, _ = self.tree.partition_matches(all_matches)

        matches_converted: Dict[int, Tuple[List[cv.DMatch], Mapping[int, cv.KeyPoint]]] = {}
        for im_id, matches in all_matches_by_im_id.items():
            dms = []
            kps = {}

            for m in matches:
                for ref in m.match_refs:
                    for image_id_, train_descriptor_id in ref.descriptor_id_by_image_id.items():
                        dm = cv.DMatch(_distance=m.distance, _imgIdx=image_id_, _queryIdx=m.query_descriptor_id,
                                       _trainIdx=train_descriptor_id)
                        kp = self.get_keypoint(im_id, train_descriptor_id)
                        dms.append(dm)
                        kps[train_descriptor_id] = kp

            matches_converted[im_id] = (dms, kps)

        return matches_converted
