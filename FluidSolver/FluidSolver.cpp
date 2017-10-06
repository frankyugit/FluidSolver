// FluidSolver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include "json/json.h"

class Point
{
public:
	Point(double x, double y, double z, int index);
	double& operator[](int i);
	friend Point operator+(Point p1, Point p2);
	friend Point operator/(Point p, double n);
	friend std::ostream& operator<<(std::ostream& osm, Point& pt);
	inline int uiIndex()
	{
		return pindex_;
	}
private:
	int pindex_;
	std::array<double, 3> coord_ = { 0,0,0 };
};

class Face
{
public:
	Face(std::vector<Point> points, int index);
	Point& operator[](int i);
	friend std::ostream& operator<<(std::ostream& osm, Face& face);
	inline int uiIndex()
	{
		return findex_;
	}
	inline int uiNpoints()
	{
		return npoints_;
	}
	double fSurfaceArea();
private:
	int findex_;
	int npoints_;
	std::vector<Point> points_;
};

class Cell
{
public:
	Cell(std::vector<Face> faces, int index);
	Face& operator[](int i);
	friend std::ostream& operator<<(std::ostream& osm, Cell& cell);
	inline int uiIndex()
	{
		return cindex_;
	}
	inline int uiNfaces()
	{
		return nfaces_;
	}
private:
	int cindex_;
	int nfaces_;
	std::vector<Face> faces_;
};


class Mesh
{
public:
	Mesh(std::string meshfile);

private:
	std::vector<Point> mpoints_;
	int Ncell_ = 0;
	int Nface_ = 0;
	int Npoint_ = 0;
	int Ninnerface_ = 0;
	void readMesh(std::string meshfile);
	void readJSONMesh(std::string meshfile);
};


Point::Point(double x, double y, double z, int index)
{
	coord_[0] = x;
	coord_[1] = y;
	coord_[2] = z;
	pindex_ = index;
}

double& Point::operator[](int i)
{
	if (i < 0 || i>2)
	{
		std::cout << "index beyonds range of point coordinate, which should between 0 and 2";
		std::abort();
	}
	else
	{
		return Point::coord_[i];
	}
}

Point operator+(Point p1, Point p2)
{
	Point ptsum(0, 0, 0, -1);
	ptsum[0] = p1[0] + p2[0];
	ptsum[1] = p1[1] + p2[1];
	ptsum[2] = p1[2] + p2[2];
	return ptsum;
}

Point operator/(Point p, double n)
{
	Point ptquo(0, 0, 0, -1);
	ptquo[0] = p[0] / n;
	ptquo[1] = p[1] / n;
	ptquo[2] = p[2] / n;
	return ptquo;
}

std::ostream& operator<<(std::ostream& osm, Point& pt)
{
	osm << "Point coord\t(" << pt[0] << " " << pt[1] << " " << pt[2] << ")\n";
	return osm;
}

std::ostream& operator<<(std::ostream& osm, Face& face)
{
	osm << "Face with " << face.npoints_ << " points\n";
	int Npoints = face.uiNpoints();
	osm << "[ ";
	for (int i = 0; i < Npoints; i++)
	{
		osm << face[i].uiIndex() << " ";
	}
	osm << "]\n";
	return osm;
}

std::ostream& operator<<(std::ostream& osm, Cell& cell)
{
	osm << "Cell with " << cell.nfaces_ << " faces\n";
	int Nfaces = cell.uiNfaces();
	osm << "[ ";
	for (int i = 0; i < Nfaces; i++)
	{
		osm << cell[i].uiIndex() << " ";
	}
	osm << "]\n";
	return osm;
}


Face::Face(std::vector<Point> points, int index)
	:npoints_(points.size()),points_(points),
	findex_(index)
{
}

double Face::fSurfaceArea()
{
	Point cenpt(0, 0, 0, -1);
	int Npt = npoints_;
	for (int i = 0; i < Npt; i++)
	{
		cenpt = cenpt + points_[i];
	}
	cenpt = cenpt / Npt;
	std::cout << "Center Point " << cenpt;
	return cenpt[0];
}

Point& Face::operator[](int i)
{
	if (i<0 || i>npoints_-1)
	{
		std::cout << "index beyonds range of point number, which should between 0 and " << (npoints_ - 1);
		std::abort();
	}
	else
	{
		return points_[i];
	}
}

Cell::Cell(std::vector<Face> faces, int index)
	:nfaces_(faces.size()), faces_(faces),
	cindex_(index)
{
}

Face& Cell::operator[](int i)
{
	if (i<0 || i>nfaces_ - 1)
	{
		std::cout << "index beyonds range of face number, which should between 0 and " << (nfaces_ - 1);
		std::abort();
	}
	else
	{
		return faces_[i];
	}
}

Mesh::Mesh(std::string meshfile)
{
	std::string Meshname = meshfile;
	readJSONMesh(Meshname);
}

void Mesh::readMesh(std::string meshfile)
{
	std::ifstream meshinput;
	meshinput.open(meshfile);
	if (!meshinput)
	{
		std::cout << "Can't read mesh file, please check";
		exit(0);
	}
	char line[256];
	int mark = 0;
	while (!meshinput.eof())
	{
		meshinput.getline(line, 256);
		std::string linestr(line);
		std::string::size_type begin = linestr.find_first_not_of(" \f\t\v");
		if (begin == std::string::npos || line[begin] == '\{' || line[begin] == '\}') continue;
		std::string::size_type end = linestr.find_last_not_of(" \f\n\r\t\v");
		std::string item = linestr.substr(begin, end + 1 - begin);

		if (item.compare("Mesh") == 0)
		{
			mark = 1;
			continue;
		}
		else if (item.compare("Points") == 0)
		{
			mark = 2;
			continue;
		}
		else if (item.compare("Faces") == 0)
		{
			mark = 3;
			continue;
		}
		else if (item.compare("Cells") == 0)
		{
			mark = 4;
			continue;
		}
		else if (item.compare("FaceCells") == 0)
		{
			mark = 5;
			continue;
		}

		int pindex = 0;

		if (mark == 1)
		{
			std::string teststr1 = linestr.substr(begin, 5);
			std::string teststr2 = linestr.substr(begin, 6);
			std::string teststr3 = linestr.substr(begin, 10);
			if (teststr1.compare("Ncell") == 0)
			{
				std::string Ncellnumstr = linestr.substr(begin + 6, end - begin - 5);
				sscanf_s(Ncellnumstr.c_str(), "%d", &Ncell_);
			}
			else if (teststr1.compare("Nface") == 0)
			{
				std::string Nfacenumstr = linestr.substr(begin + 6, end - begin - 5);
				sscanf_s(Nfacenumstr.c_str(), "%d", &Nface_);
			}
			else if (teststr2.compare("Npoint") == 0)
			{
				std::string Npointnumstr = linestr.substr(begin + 7, end - begin - 6);
				sscanf_s(Npointnumstr.c_str(), "%d", &Npoint_);
			}
			else if (teststr3.compare("Ninnerface") == 0)
			{
				std::string Ninnerface = linestr.substr(begin + 11, end - begin - 10);
				sscanf_s(Ninnerface.c_str(), "%d", &Ninnerface_);
			}
		}
		else if (mark == 2)
		{
			if (linestr[begin] == '\(')
			{
				int numlocal1 = linestr.find(",");
				int numlocal2 = linestr.find(",", numlocal1 + 1);
				int numlocal3 = end - 1;
				double pCoordx = 0;
				double pCoordy = 0;
				double pCoordz = 0;
				std::string num1 = linestr.substr(begin + 1, numlocal1 - begin - 1);
				std::string num2 = linestr.substr(numlocal1 + 1, numlocal2 - numlocal1 - 1);
				std::string num3 = linestr.substr(numlocal2 + 1, numlocal3 - numlocal2);
				sscanf_s(num1.c_str(), "%lf", &pCoordx);
				sscanf_s(num2.c_str(), "%lf", &pCoordy);
				sscanf_s(num3.c_str(), "%lf", &pCoordz);
				Point pt(pCoordx, pCoordy, pCoordz, pindex);
				mpoints_.push_back(pt);
				pindex++;

			}

		}
	}
	meshinput.close();
}


void Mesh::readJSONMesh(std::string meshfile)
{
	Json::Value root;
	std::ifstream meshdata(meshfile, std::ifstream::binary);
	meshdata >> root;
	const Json::Value MeshConstants = root["Mesh"];

	Ncell_ = MeshConstants["Ncell"].asInt();
	Nface_ = MeshConstants["Nface"].asInt();
	Npoint_ = MeshConstants["Npoint"].asInt();
	Ninnerface_ = MeshConstants["Ninnerface"].asInt();

}

int main()
{
	Point pointtest(0.1, 0.2, 0.3, 0);
	std::cout << "x=" << pointtest[0] << "\n";
	std::cout << "y=" << pointtest[1] << "\n";
	std::cout << "z=" << pointtest[2] << "\n";
	std::cout << pointtest;
	system("pause");
	Point pointtest2(0.2, 0.3, 0.4, 1);
	Point pointtest3(0.3, 0.4, 0.5, 2);
	Point pointtest4(0.4, 0.5, 0.6, 3);
	std::vector<Point> ptvtr;
	ptvtr.push_back(pointtest);
	ptvtr.push_back(pointtest2);
	ptvtr.push_back(pointtest3);
	std::vector<Point> ptvtr2;
	ptvtr2.push_back(pointtest2);
	ptvtr2.push_back(pointtest3);
	ptvtr2.push_back(pointtest4);
	Face facetest(ptvtr, 0);
	Face facetest2(ptvtr2, 1);
	std::vector<Face> fvtr;
	fvtr.push_back(facetest);
	fvtr.push_back(facetest2);
	Cell celltest(fvtr, 0);
	std::cout << celltest;
	Face ftest = celltest[1];
	std::cout << ftest;
	Point facepttest = ftest[1];
	std::cout << facepttest;
	system("pause");
	std::cout << "Point sum test\n";
	std::cout << facetest.fSurfaceArea() << "\n";

	std::string meshfile = "./mesh.json";
	Mesh meshtest(meshfile);
	std::cin.get();
	return 0;
}

