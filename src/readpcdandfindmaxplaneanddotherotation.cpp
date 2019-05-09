#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>
 
#include <pcl/ModelCoefficients.h> 
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>  
#include <string>
#include <cmath>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h> 
#include <pcl/io/png_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/cloud_viewer.h>
 
#include <sensor_msgs/PointCloud2.h>

#include <pcl/point_cloud.h>
#include <pcl/common/transforms.h>              
#include <ros/ros.h>
// PCL 的相关的头文件
#include <sensor_msgs/PointCloud2.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
//滤波的头文件
#include "readtopictopointnet/rospcdmsg.h" 
#include <pcl/filters/voxel_grid.h>
//申明发布器

using readtopictopointnet::rospcdmsg;

ros::Publisher pub;

typedef pcl::PointXYZRGBA  PointT;


void CrossProduct(double a[3], double b[3], double ret[3])
{
    ret[0] = a[1] * b[2] - a[2] * b[1];
    ret[1] = a[2] * b[0] - a[0] * b[2];
    ret[2] = a[0] * b[1] - a[1] * b[0];
}

double DotProduct(double a[3], double b[3])
{
    double result;
    result = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];

    return result;
}

double Normalize(double v[3])
{
    double result;

    result = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

    return result;
}

void Rotate(double** rotateMatrix, double u[3], double ret[3]){
    ret[0]=rotateMatrix[0][0]*u[0]+rotateMatrix[0][1]*u[1]+rotateMatrix[0][2]*u[2];
    ret[1]=rotateMatrix[1][0]*u[0]+rotateMatrix[1][1]*u[1]+rotateMatrix[1][2]*u[2];
    ret[2]=rotateMatrix[2][0]*u[0]+rotateMatrix[2][1]*u[1]+rotateMatrix[2][2]*u[2];
}

void RotationMatrix(double angle, double u[3], double rotatinMatrix[3][3])
{
    double norm = Normalize(u);
    
    u[0] = u[0] / norm;
    u[1] = u[1] / norm;
    u[2] = u[2] / norm;

    rotatinMatrix[0][0] = cos(angle) + u[0] * u[0] * (1 - cos(angle));
    rotatinMatrix[0][1] = u[0] * u[1] * (1 - cos(angle) - u[2] * sin(angle));
    rotatinMatrix[0][2] = u[1] * sin(angle) + u[0] * u[2] * (1 - cos(angle));

    rotatinMatrix[1][0] = u[2] * sin(angle) + u[0] * u[1] * (1 - cos(angle));
    rotatinMatrix[1][1] = cos(angle) + u[1] * u[1] * (1 - cos(angle));
    rotatinMatrix[1][2] = -u[0] * sin(angle) + u[1] * u[2] * (1 - cos(angle));
      
    rotatinMatrix[2][0] = -u[1] * sin(angle) + u[0] * u[2] * (1 - cos(angle));
    rotatinMatrix[2][1] = u[0] * sin(angle) + u[1] * u[2] * (1 - cos(angle));
    rotatinMatrix[2][2] = cos(angle) + u[2] * u[2] * (1 - cos(angle));

}

//cal transport
void Calculation3d(double vectorBefore[3], double vectorAfter[3], double rotatinMatrix[3][3])
{
    double  rotationAxis[3];
    double rotationAngle;
    CrossProduct(vectorBefore, vectorAfter, rotationAxis);
    rotationAngle = acos(DotProduct(vectorBefore, vectorAfter) / Normalize(vectorBefore) / Normalize(vectorAfter));
    RotationMatrix(rotationAngle, rotationAxis, rotatinMatrix);
}

void Calculation4d(double vectorBefore[3], double vectorAfter[3], double rotatinMatrix[4][4])
{
    double rotate3d[3][3];
	Calculation3d(vectorBefore,vectorAfter, rotate3d);

	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
			rotatinMatrix[i][j] = rotate3d[i][j];

	for(int i = 0; i < 3; i++)
	{
		rotatinMatrix[i][3] = 0;
		rotatinMatrix[3][i] = 0;
	}

	rotatinMatrix[3][3] = 1;
}

bool sendpointcloud(rospcdmsg::Request& request, rospcdmsg::Response& response)
{
  //sensor_msgs::convertPointCloud2ToPointCloud(request.pointcloud2, response.pointcloud);

  return true;
}

void 
cloud_cb (const sensor_msgs::PointCloud2ConstPtr& input)  //特别注意的是这里面形参的数据格式
{
 sensor_msgs::PointCloud2 input2; 

input2 = *input;
  
  pcl::PointCloud<pcl::PointXYZ>::Ptr beforecloud (new pcl::PointCloud<pcl::PointXYZ>);
pcl::PointCloud<pcl::PointXYZ>::Ptr  cloud (new pcl::PointCloud<pcl::PointXYZ>);
pcl::PointCloud<pcl::PointXYZ>::Ptr cuttedcloud (new pcl::PointCloud<pcl::PointXYZ>);
    pcl::fromROSMsg(input2,*beforecloud);

 
 
  // draw the point cloud
  //boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer (new pcl::visualization::PCLVisualizer ("3D Viewer"));
  //pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGBA> rgb(cloud);
  //viewer->addPointCloud<pcl::PointXYZRGBA>(cloud, rgb, "registered point cloud");
  //viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 3, "registered point cloud");

  // draw the samples
 


  // Create the filtering object
    pcl::VoxelGrid<pcl::PointXYZ> sor;
    sor.setInputCloud(beforecloud);
    sor.setLeafSize(0.01f, 0.01f, 0.01f);
    sor.filter(*cloud);
 




  pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
  pcl::PointIndices::Ptr inliers (new pcl::PointIndices);
  // Create the segmentation object
  pcl::SACSegmentation<pcl::PointXYZ> seg;
  // Optional
  seg.setOptimizeCoefficients (true);
  // Mandatory
  seg.setModelType (pcl::SACMODEL_PLANE);
  seg.setMethodType (pcl::SAC_RANSAC);
  seg.setDistanceThreshold (0.002);

  seg.setInputCloud (cloud);
  seg.segment (*inliers, *coefficients);
  // This is also where we specify the “distance threshold”, which determines how close a point must be to the model in order to be considered an inlier. 
  if (inliers->indices.size () == 0)
  {
    PCL_ERROR ("Could not estimate a planar model for the given dataset.");
 
  }

  std::cerr << "Model coefficients: " << coefficients->values[0] << " " 
                                      << coefficients->values[1] << " "
                                      << coefficients->values[2] << " " 
                                      << coefficients->values[3] << std::endl;
// the estimated plane parameters  ax+by+cz+d=0
 
 


  // draw the samples
 // viewer->addPointCloud<pcl::PointXYZ>(filtered_cloud, "samples cloud2");
 // viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 4, "samples cloud2");
 // viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 1.0, 0.0,0.0, "samples cloud2");
 // viewer->addPointCloud<pcl::PointXYZ>( cloud, "samples cloudori");
  //viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "samples cloudori");
 // viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 1.0, 1.0,1.0, "samples cloudori");
//viewer->addCoordinateSystem (0.1);
//while (!viewer->wasStopped ())
 // {
 //   viewer->spinOnce (100);
  //  boost::this_thread::sleep (boost::posix_time::microseconds (100000));
 // }
 
 
 // if( coefficients->values[0]*cloud->points[i].x+coefficients->values[1]*cloud->points[i].y+coefficients->values[2]*cloud->points[i].z+coefficients->values[3]  >-0.06
//and  coefficients->values[0]*cloud->points[i].x+coefficients->values[1]*cloud->points[i].y+coefficients->values[2]*cloud->points[i].z+coefficients->values[3] <-0.005


    // 创建存储点云重心的对象
    Eigen::Vector4f centroid;
    
    pcl::compute3DCentroid(*cloud, centroid);

    std::cout << "The XYZ coordinates of the centroid are: ("
              << centroid[0] << ", "
              << centroid[1] << ", "
              << centroid[2] << ")." << std::endl;
  
	double vectorBefore[3]  = {coefficients->values[0],coefficients->values[1],coefficients->values[2]};
	double vectorAfter[3] = {0,0,-1};
	double rotatinMatrix[4][4];
	Calculation4d(  vectorBefore , vectorAfter ,   rotatinMatrix );
	std::cout << vectorBefore[0]<<"," << vectorBefore[1]<<"," << vectorBefore[2]<<std::endl;
	std::cout << rotatinMatrix[0][0]<<"," << rotatinMatrix[0][1]<<"," << rotatinMatrix[0][2]<<"," << rotatinMatrix[0][3]<<std::endl;
	std::cout << rotatinMatrix[1][0]<<"," << rotatinMatrix[1][1]<<"," << rotatinMatrix[1][2]<<"," << rotatinMatrix[1][3]<<std::endl;
	std::cout << rotatinMatrix[2][0]<<"," << rotatinMatrix[2][1]<<"," << rotatinMatrix[2][2]<<"," << rotatinMatrix[2][3]<<std::endl;
	std::cout << rotatinMatrix[3][0]<<"," << rotatinMatrix[3][1]<<"," << rotatinMatrix[3][2]<<"," << rotatinMatrix[3][3]<<std::endl; 
	 
     // pcl::PointXYZ  point;
     // point.x = centroid[0];
     // point.y = centroid[1];
    //  point.z = (-coefficients->values[3]-( coefficients->values[0]*centroid[0]+coefficients->values[1]*centroid[1]))/coefficients->values[2];
//singlepoint->points.push_back (point);
   // transform_2 (0,3) = centroid[0];
  //  transform_2 (1,3) = centroid[1];
   // transform_2 (2,3) = (-coefficients->values[3]-( coefficients->values[0]*centroid[0]+coefficients->values[1]*centroid[1]))/coefficients->values[2];
 


    Eigen::Matrix4f transform_1 = Eigen::Matrix4f::Identity();
    Eigen::Vector4f vec4d(centroid[0],centroid[1], (-coefficients->values[3]-( coefficients->values[0]*centroid[0]+coefficients->values[1]*centroid[1]))/coefficients->values[2], 1.0);  
    // Define a rotation matrix (see https://en.wikipedia.org/wiki/Rotation_matrix)
    // Here we defined a 45° (PI/4) rotation around the Z axis and a translation on the X axis.
  //  float theta = -M_PI/4; // The angle of rotation in radians
    transform_1 (0,0) = rotatinMatrix[0][0]; 
    transform_1 (0,1) = rotatinMatrix[0][1]; 
    transform_1 (0,2) = rotatinMatrix[0][2]; 
    transform_1 (0,3) = rotatinMatrix[0][3]; 
    transform_1 (1,0) = rotatinMatrix[1][0]; 
    transform_1 (1,1) = rotatinMatrix[1][1]; 
    transform_1 (1,2) = rotatinMatrix[1][2]; 
    transform_1 (1,3) = rotatinMatrix[1][3]; 
    transform_1 (2,0) = rotatinMatrix[2][0]; 
    transform_1 (2,1) = rotatinMatrix[2][1]; 
    transform_1 (2,2) = rotatinMatrix[2][2]; 
    transform_1 (2,3) = rotatinMatrix[2][3]; 
    transform_1 (3,0) = rotatinMatrix[3][0]; 
    transform_1 (3,1) = rotatinMatrix[3][1]; 
    transform_1 (3,2) = rotatinMatrix[3][2]; 
    transform_1 (3,3) = rotatinMatrix[3][3]; 
    //    (row, column)

    // Define a translation of 2.5 meters on the x axis.


    // Print the transformation
    printf ("Method #1: using a Matrix4f\n");
    std::cout << transform_1 << std::endl;

	Eigen::Vector4f  newvec4d;
	newvec4d=transform_1*vec4d;

    Eigen::Matrix4f transform_2 = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f transform_3= Eigen::Matrix4f::Identity();
    transform_2 (0,3) = -newvec4d[0];
     transform_2 (1,3) = -newvec4d[1];
     transform_2 (2,3) = -newvec4d[2];
 


    transform_3=transform_2*transform_1;
    // Executing the transformation
    pcl::PointCloud<pcl::PointXYZ>::Ptr transformed_cloud (new pcl::PointCloud<pcl::PointXYZ> ());
    /*
    void pcl::transformPointCloud(const pcl::PointCloud< PointT > & cloud_in, 
                                    pcl::PointCloud< PointT > &  cloud_out,  
                                    const Eigen::Matrix4f &  transform  ) 
    */
    // Apply an affine transform defined by an Eigen Transform.

 


    pcl::transformPointCloud (*cloud, *transformed_cloud, transform_3);


  for (size_t i = 0; i < transformed_cloud->points.size (); ++i)
  {	
	pcl::PointXYZ  point;
if (transformed_cloud->points[i].x <0.3 and transformed_cloud->points[i].x >-0.3 and transformed_cloud->points[i].y <0.3 and transformed_cloud->points[i].y >-0.3)
   { point.x=transformed_cloud->points[i].x  ;
    point.y=transformed_cloud->points[i].y  ;
    point.z=transformed_cloud->points[i].z ;

	cuttedcloud->points.push_back (point);}
 
  }


     // pcl::PointXYZ  point;
     // point.x = centroid[0];
     // point.y = centroid[1];
    //  point.z = (-coefficients->values[3]-( coefficients->values[0]*centroid[0]+coefficients->values[1]*centroid[1]))/coefficients->values[2];
//singlepoint->points.push_back (point);

//这是一个失败的程序
//旋转并不能让点云生成的深度图也改变
//因为点云里面点的顺序并没有改变，变的只是这些点的值





 std::cout << "downsampling: " <<cuttedcloud->points.size () << std::endl;


 //std::string depth_file("/home/yuan/doc/suction_ws/src/gqcnn-dev_jeff/outputnewrotation.png");
 //  saveDepthToPNG(depth_file, *transformed_cloud);
 
//boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer (new pcl::visualization::PCLVisualizer ("3D Viewer"));
  // draw the samples
 //viewer->addPointCloud<pcl::PointXYZ>(cuttedcloud, "samples cloud2");
 // viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 4, "samples cloud2");
  //viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 1.0, 1.0,1.0, "samples cloud2");

 //viewer->addCoordinateSystem ( 1);
 //while (!viewer->wasStopped ())
 //{
  //  viewer->spinOnce (100);
  //boost::this_thread::sleep (boost::posix_time::microseconds (100000));
 //}
 
    // save depth image

if (cuttedcloud->points.size () >0) 
{
	ros::NodeHandle n;   //声明节点的名称
	ros::ServiceClient client = n.serviceClient<readtopictopointnet::rospcdmsg>("send_point_cloud");//("send_point_cloud", sendpointcloud);
	readtopictopointnet::rospcdmsg srv;


	sensor_msgs::PointCloud2 sendmsg;
	pcl::toROSMsg(*cuttedcloud,sendmsg);

	srv.request.cloud_in=sendmsg;

	client.call(srv);

}


}
















int
main (int argc, char** argv)
{
  // 初始化 ROS节点
  ros::init (argc, argv, "my_pcl_tutorial");
  ros::NodeHandle nh;   //声明节点的名称

  // 为接受点云数据创建一个订阅节点
  ros::Subscriber sub = nh.subscribe ("/camera/depth_registered/points", 1, cloud_cb);

  //创建ROS的发布节点
  pub = nh.advertise<sensor_msgs::PointCloud2> ("output_result", 1);

  // 回调
  ros::spin ();
}

