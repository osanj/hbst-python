import time

import cv2 as cv

from matcher import BruteForceMatcher, HbstMatcher


def run() -> None:
    train_paths = ["images/graf1.png", "images/leuvenA.jpg"]
    test_paths = ["images/leuvenB.jpg"]

    orb = cv.ORB(1000)

    train_features = {}
    for i, path in enumerate(train_paths):
        im = cv.imread(path)
        kps, descs = orb.detectAndCompute(im, mask=None)
        train_features[i] = (kps, descs)

    matchers = [BruteForceMatcher(train_features),
                HbstMatcher(orb.descriptorSize(), train_features)]

    for path in test_paths:
        im = cv.imread(path)
        _, query_descs = orb.detectAndCompute(im, mask=None)

        for matcher in matchers:
            t0 = time.time()
            matches = matcher.match(query_descs)
            dt = time.time() - t0

            print(f"{matcher.__class__.__name__} took {dt:.3f} seconds")
            counts_by_im_id = {im_id: len(ms) for im_id, (ms, _) in matches.items()}
            for im_id, count in sorted(counts_by_im_id.items(), key=lambda x: -x[1]):
                print(f"{count} matches with {train_paths[im_id]}")
            print()


if __name__ == "__main__":
    run()
