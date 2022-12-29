import kdtree
import numpy as np


def distance(point1, point2):
    return sum([(point1[i] - point2[i]) ** 2 for i in range(3)]) ** 0.5


num = 0
points_GSM = np.loadtxt("cmake-build-debug/output/GSM_" + str(num))
points_LSM = np.loadtxt("cmake-build-debug/output/LSM_" + str(num))

points_GSM = points_GSM[:, :3].tolist()
points_LSM = points_LSM[:, :3].tolist()

Tree = kdtree.create(point_list=points_GSM, dimensions=3)

point_idx = list(range(len(points_LSM)))
paired_map = {}

while len(point_idx) != 0:
    i = point_idx[-1]
    point_idx.pop(-1)
    print(len(point_idx))

    result = Tree.search_nn_dist(point=(points_LSM[i]), distance=1)

    for j in range(len(result)):
        idx = points_GSM.index(result[j])
        if not idx in paired_map:
            paired_map[idx] = [i, distance(points_LSM[i], result[j])]
        else:
            paired_pts = paired_map[idx]
            if distance(points_LSM[i], result[j]) < paired_pts[1]:
                point_idx.append(paired_pts[0])
                paired_map[idx] = [i, distance(points_LSM[i], result[j])]
            else:
                continue
exit(0)
