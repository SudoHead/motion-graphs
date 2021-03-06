#include "../headers/Animation.h"

Animation::Animation(Skeleton* sk, char* amc_filename)
{
	//initialize first pose at index 0 (frame 1)

	bool started = false; //for skipping the first few lines of header stuff

	Pose* pose = new Pose(1);

	string line;
	ifstream myfile(amc_filename);
	int c = 0;

	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			/*	Loading data	*/
			// cout << "loading line : " << line << "\n";
			vector<string> toks = strSplit(line, ' ');
			
			long p_frame = is_new_frame(line);
			if (p_frame > -1) {
				//push the frame
				if (started) {
					//cout << "--- adding pose #" << p_frame << "\n";
					c++;
					this->poses.push_back(pose);
					pose = new Pose(p_frame);
				}
				else {
					started = true;
				}
			}
			if (started && p_frame == -1) //if started and current line is not frame number
			{
				vector<float> t;
				for (int i = 1; i < toks.size(); i++) {
					t.push_back(stod(toks.at(i)));
				}
				string bone_name = toks.at(0);
				Bone * bone_ = sk->getByName(bone_name);
				pose->addTransf(bone_name, t, bone_->dof[0], bone_->dof[1], bone_->dof[2]);
			}
		}
		//adding last frame/pose
		if (started)
			this->poses.push_back(pose);
		myfile.close();
	}
	else std::cout << "Unable to open file";

	std::cout << "\nAnimation: added "<< c <<  " poses\n";
}

Animation::Animation(vector<Pose*> ps)
{
	this->poses.reserve(ps.size());
	for (auto &p:ps) {
		// Pose* pose = new Pose(p->getPoseFrame());
		this->poses.emplace_back(new Pose(*p));
	}
}

Animation::Animation(const Animation &anim)
{
	this->poses.reserve(anim.poses.size());
	for (auto &p:anim.poses) {
		// Pose* pose = new Pose(p->getPoseFrame());
		this->poses.emplace_back(new Pose(*p));
	}
	this->currentFrame = anim.currentFrame;
}

void Animation::addPoses(vector<Pose*> ps) 
{
	this->poses.insert(this->poses.end(), ps.begin(), ps.end());
}

Pose* Animation::getPoseAt(long frame)
{
	this->currentFrame = frame;
	frame--; // poses start from [0], frames starts from 1
	if (this->poses.size() > frame && frame >= 0) {
		return this->poses.at(frame);
	}
	else 
		return NULL;
}

void Animation::setPoseAt(long frame, Pose *pose)
{
	if (this->poses.size() > frame && frame >= 0) {
		this->poses.at(frame) = pose;
	}
}

Pose* Animation::getNextPose()
{
	if (currentFrame > poses.size() - 1)
		return NULL;
	else {
		//cout << "--- Returning pose at frame " << currentFrame << "\n";
		Pose* p = this->poses.at(currentFrame);
		this->currentFrame++;
		return p;
	}
}

vector<Pose*> Animation::getPosesInRange(unsigned long start, unsigned long end)
{
	vector<Pose*> ps;
	if (start >= 0 && start <= this->poses.size() && start < end && end >= 0 && end <= this->poses.size()) {
		for(int i = start; i <= end; i++) {
			ps.push_back(this->getPoseAt(i));
		}
	} else {
		cout << "Animation::getPosesInRange: range error -> start = " 
		<< start << ", end = " << end << ", size = " << this->poses.size() << endl;
	}
	return ps;
}

vector<Pose*> Animation::getAllPoses()
{
	return this->poses;
}

long Animation::getCurrentFrame()
{
	return this->currentFrame;
}

long Animation::getNumberOfFrames() 
{
	return this->poses.size();
}

void Animation::setFrame(long frame)
{
	if (frame > 0 && frame < this->poses.size())
		this->currentFrame = frame;
}

bool Animation::isOver()
{
	return currentFrame > this->poses.size() - 1;
}

void Animation::reset()
{
	this->currentFrame = 1;
}

void Animation::normalise(Pose* n_pose, long frame)
{
	string root = "root";
	// normalise all to frame k
	if (frame >= 0 && frame < poses.size()) {
		glm::vec3 j_pos_relative = this->poses.at(frame)->getRootPos();
		glm::quat j_quat_relative = this->poses.at(frame)->getBoneTrans(root);
		glm::quat n_quat = n_pose->getBoneTrans(root);
		glm::vec3 euler_ang = glm::eulerAngles(n_quat);
		glm::mat4 R = glm::mat4(1.f);
		R = glm::rotate(R, euler_ang.y, glm::vec3(0.0f, 1.f, 0.f));
		glm::quat xz_n_quat = glm::quat_cast(R);

		j_pos_relative = glm::vec3(glm::mat4_cast(n_quat) * glm::vec4(j_pos_relative, 1.f));
		for (long i = 0; i < poses.size(); i++) {
			glm::vec3 old_pos = this->poses.at(i)->getRootPos();
			glm::vec3 pos = glm::vec3(glm::mat4_cast(n_quat) * glm::vec4(old_pos, 1.f));
			// if (i <= frame) {
			// 	pos = n_pose->getRootPos() + (j_pos_relative - this->poses.at(i)->getRootPos());
			// } else 
			// 	pos = n_pose->getRootPos() - (j_pos_relative - this->poses.at(i)->getRootPos());
			pos = n_pose->getRootPos() - (j_pos_relative - pos);
			poses.at(i)->set_pos(glm::vec3(pos.x, old_pos.y, pos.z));

			glm::quat qa = this->poses.at(i)->getBoneTrans(root);
			glm::quat quat = glm::normalize(qa * xz_n_quat);
			poses.at(i)->set_rot(root, quat);
		}
	} else {
		cout << "Error: Aniamtion: normalise(). Frame out of range.";
	}
}

Animation::~Animation() 
{
	for (auto pose: this->poses)
		delete pose;
}