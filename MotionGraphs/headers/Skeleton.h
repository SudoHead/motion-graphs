#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "Bone.h"
#include "Pose.h"
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "Utils.h"

#include "../includes/imgui/imgui.h"
#include "../includes/imgui/imgui_impl_glfw.h"
#include "../includes/imgui/imgui_impl_opengl3.h"

using namespace std;

class Skeleton :
	public Bone
{
public:
	glm::vec3 root_offset = glm::vec3(0.f);
	//root bone
	string order = "TX TY TZ RX RY RZ"; //default
	string axiz_order = "XYZ"; //default

	Skeleton();
	Skeleton(char* asf_filename, float scale);
	Skeleton(const Skeleton &skel);
	
	Bone* getByName(string name);
	Bone* getById(int id);
	int getIdByName(string name);

	void apply_pose(Pose* pose);
	void apply_pose_locked(Pose* pose);
	glm::vec3 getPos();
	vector<Bone*> getAllBones();
	void resetAll();

	void rescale(float scale);

	map<string, PointCloud*> getBoneWindowPointCloud(vector<Pose*> poses);
	PointCloud* getGlobalWindowPointCloud(vector<Pose*> poses);
	PointCloud* getGlobalPointCloud(Pose* pose);

	~Skeleton();

protected:
	//other bones
	vector<Bone*> bones;


	void apply_pose_to_bones(Pose *pose);
};