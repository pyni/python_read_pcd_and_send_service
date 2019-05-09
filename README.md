# python_read_pcd_and_send_service

This is to read camera topic and transform the point cloud topic into the center automatically.


Then it will send the topic by rosservice

这份代码还有的点就是如何自定义关于点云的service

if you want to receive the point cloud: you can use following code:

from readtopictopointnet.srv import * 


def dealwithpcd(req):
    cloud_points = list(point_cloud2.read_points(req.cloud_in, skip_nans=True, field_names = ("x", "y", "z")))
    print ('cloud_points',cloud_points )
    
       
    
s = rospy.Service('send_point_cloud', rospcdmsg, dealwithpcd)
