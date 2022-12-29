import open3d
import matplotlib
import numpy as np

num = 1
points_GSM = np.loadtxt("cmake-build-debug/output/GSM_" + str(num))
points_LSM = np.loadtxt("cmake-build-debug/output/LSM_" + str(num))

vis = open3d.visualization.Visualizer()
vis.create_window()

vis.get_render_option().point_size = 4.0
vis.get_render_option().background_color = np.zeros(3)

pts_gsm = open3d.geometry.PointCloud()
pts_gsm.points = open3d.utility.Vector3dVector(points_GSM[:, :3])
pts_gsm.paint_uniform_color([1, 1, 1])

pts_lsm = open3d.geometry.PointCloud()
pts_lsm.points = open3d.utility.Vector3dVector(points_LSM[:, :3])
pts_lsm.paint_uniform_color([1, 0, 0])

icp = open3d.pipelines.registration.registration_icp(
    source=pts_lsm,
    target=pts_gsm,
    max_correspondence_distance=0.05,    # 距离阈值
    estimation_method=open3d.pipelines.registration.TransformationEstimationPointToPoint()
)

# pts_lsm.transform(icp.transformation)

vis.add_geometry(pts_gsm)
vis.add_geometry(pts_lsm)

vis.run()
vis.destroy_window()