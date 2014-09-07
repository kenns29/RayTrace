/*
  NOTE: This is the file you will need to begin working with
		You will need to implement the RayTrace::CalculatePixel () function

  This file defines the following:
	RayTrace Class
*/

#ifndef RAYTRACE_H
#define RAYTRACE_H

#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Utils.h"


/*
	RayTrace Class - The class containing the function you will need to implement

	This is the class with the function you need to implement
*/

class RayTrace
{
public:
	/* - Scene Variable for the Scene Definition - */
	Scene m_Scene;

	// -- Constructors & Destructors --
	RayTrace (void) {}
	~RayTrace (void) {}

	void setRayTrace(){
			// Get the Background
		sceneBg = m_Scene.GetBackground ();

		// Get the Lights (OpenGL will only support up to 8)
		numLights = m_Scene.GetNumLights ();
		for (unsigned int n = 0; n < numLights && n < 8; n++){
			sceneLight[n] = m_Scene.GetLight (n);
		}
		
		// Get the Camera
		sceneCam = m_Scene.GetCamera ();

		// Get the Objects and Render them
		numObjs = m_Scene.GetNumObjects ();
	}
	// -- Main Functions --
	// - CalculatePixel - Returns the Computed Pixel for that screen coordinate
	Vector CalculatePixel (int screenX, int screenY)
	{
		/*
			-- How to Implement a Ray Tracer --

			This computed pixel will take into account the camera and the scene
				and return a Vector of <Red, Green, Blue>, each component ranging from 0.0 to 1.0

			In order to start working on computing the color of this pixel,
				you will need to cast a ray from your current camera position
				to the image-plane at the given screenX and screenY
				coordinates and figure out how/where this ray intersects with 
				the objects in the scene descriptor.
				The Scene Class exposes all of the scene's variables for you 
				through its functions such as m_Scene.GetBackground (), m_Scene.GetNumLights (), 
				etc. so you will need to use those to learn about the World.

			To determine if your ray intersects with an object in the scene, 
				you must calculate where your objects are in 3D-space [using 
				the object's scale, rotation, and position is extra credit]
				and mathematically solving for an intersection point corresponding to that object.
			
			For example, for each of the spheres in the scene, you can use 
				the equation of a sphere/ellipsoid to determine whether or not 
				your ray from the camera's position to the screen-pixel intersects 
				with the object, then from the incident angle to the normal at 
				the contact, you can determine the reflected ray, and recursively 
				repeat this process capped by a number of iterations (e.g. 10).

			Using the lighting equation to calculate the color at every 
				intersection point and adding its fractional amount (determined by the material)
				will get you a final color that returns to the eye at this point.
		*/


		if ((screenX < 0 || screenX > WINDOW_WIDTH - 1) ||
			(screenY < 0 || screenY > WINDOW_HEIGHT - 1))
		{
			// Off the screen, return black
			return Vector (0.0f, 0.0f, 0.0f);
		}


		// Until this function is implemented, return white
		Vector viewVector = sceneCam.GetTarget() - sceneCam.GetPosition();
		Vector sideVector = sceneCam.GetUp().Cross(viewVector);
		Vector upVector = viewVector.Cross(sideVector).Normalize();
		float height = viewVector.Magnitude()* tan(sceneCam.GetFOV()/2.0 * M_PI / 180.0);
		float width = (GLdouble)WINDOW_WIDTH/WINDOW_HEIGHT * height;
		float y = (float)screenY / WINDOW_HEIGHT * ( 2 * height);
		float x = (float)screenX / WINDOW_WIDTH * (2 * width);
		Vector firstRay = viewVector + upVector * (-height + y) + sideVector.Normalize() * (width - x);
		firstRay = firstRay.Normalize();
		firstRay.w = 1.0;
		
		//printf("( %1.1f, %1.1f, %1.1f, %1.1f)\n", firstRay.x, firstRay.y, firstRay.z, firstRay.w);
		Vector color = trace(sceneCam.GetPosition() ,firstRay, 1);
		return color;
	}

private:
	SceneBackground sceneBg;
	unsigned int numLights;
	SceneLight sceneLight[8];
	Camera sceneCam;
	unsigned int numObjs;
	/* calculate the perfect reflector */
	Vector reflect(Vector ray, Vector normal){
		return (normal * 2.0 * ray.Dot(normal)- ray).Normalize();
	}

	/*clamp */
	Vector clamp(Vector v){
		Vector vect = v;
		if (v.x < 0.0) vect.x = 0.0;
		else if(v.x > 1.0) vect.x = 1.0;
		if (v.y< 0.0) vect.y = 0.0;
		else if(v.y > 1.0) vect.y = 1.0;
		if (v.z < 0.0) vect.z = 0.0;
		else if(v.z > 1.0) vect.z = 1.0;
		if (v.w < 0.0) vect.w = 0.0;
		else if(v.w > 1.0) vect.w = 1.0;
		return vect;
	}
	/*trace*/
	Vector trace(Vector rayOrigin, Vector ray, int N){
		if(N > 10)
			return Vector(0.0, 0.0, 0.0, 0.0);
		bool isInt = false;
		Vector intersection;
		Vector normal;
		SceneMaterial sceneMat;
		SceneSquare *board;
		string objName = "";
		float t = 0.0;

		for(int i = 0; i < numObjs; i++){
			SceneObject *obj = m_Scene.GetObject (i);
			if(isIntersection(rayOrigin, ray, obj, intersection, normal, sceneMat, t, objName ,false)){
				if(sceneMat.name == "checkerboard"){
					board = (SceneSquare*)obj;
				}
				isInt = true;
			}
		}
		if(isInt){
			Vector refl = reflect(ray, normal);
			
			Vector color = Vector(0.0, 0.0, 0.0, 0.0);
			for(int i = 0; i < numLights; i++){
				Vector lightVec = (sceneLight[i].position - intersection).Normalize();
				Vector lightColor = sceneLight[i].color;
			
				bool isShadow = false;
				for(int i = 0; i < numObjs; i++){
					SceneObject *obj = m_Scene.GetObject (i);
					Vector shadowIntersect;
					Vector n;
					float ttt = 0;
					SceneMaterial s_m;
					if(isIntersection(intersection+lightVec*0.001, lightVec, obj, shadowIntersect, n, s_m, ttt, objName ,false)){
						isShadow = true;
				
						break;
					}
				}
				if(sceneMat.name == "checkerboard"){
					float width = board->vertex[1].x - board->vertex[0].x;
					float height = -(board->vertex[3].z - board->vertex[0].z);
					float x_dis = -board->vertex[0].x;
					float z_dis = -board->vertex[0].z;
					bool wid_is_even = ((int)((intersection.x + x_dis)/width * 7.0) % 2) == 0;
					bool hei_is_even = ((int)((intersection.z + z_dis)/height * 7.0) % 2) == 0;
					if((wid_is_even && hei_is_even) || (!wid_is_even && !hei_is_even) )
						color = color + Vector(0.5, 0.5, 0.5, 0.0) * (!isShadow);
					else
						color = color + Vector(0.0, 0.0, 0.0, 0.0) * (!isShadow);
				}
				else
					color = color + (sceneMat.diffuse * lightColor * (float)max(lightVec.Dot(normal),0.0)+ sceneMat.specular * lightColor * pow((double)max(refl.Dot(ray.Normalize()), 0.0), (double)sceneMat.shininess)) * (!isShadow);
			}
			//color = sceneMat.diffuse * (float)max((sceneLight[0].position - intersection).Normalize().Dot(normal),0.0)+ sceneMat.specular * pow((double)max(refl.Dot(ray.Normalize()), 0.0), (double)sceneMat.shininess);
			//printf("sceneLight = (%1.2f, %1.2f, %1.2f, %1.2f)\n", sceneLight[0].position.x, sceneLight[0].position.y, sceneLight[0].position.z, sceneLight[0].position.w);
			//printf("intersection = (%1.2f, %1.2f, %1.2f, %1.2f)\n", intersection.x, intersection.y, intersection.z, intersection.w);
			//printf("x = %1.2f\n", x);
			float reflect_coeff = 0.3;
			if(objName == "Sphere4" || objName == "Sphere5"){
				reflect_coeff = 0.7;
			}
			float refract_coeff = 0.0;
			if(objName == "Sphere6"){
				refract_coeff = 0.9;
			}
			color = color + trace(intersection + refl*-0.001, refl* -1.0, N+1) * reflect_coeff + trace(intersection + ray * 0.001, ray, N+1) * refract_coeff;
			color = clamp(color);
			return Vector(color.x, color.y, color.z, 1.0);
		}
		else{
			return sceneBg.color;
		}
	}

	/*detect intersection */
	bool isIntersection(Vector rayOrigin, Vector ray, SceneObject *obj, Vector& intersection, Vector &norm, SceneMaterial& s_mat, float &tt, string &objName, bool debug){
		bool isInt = false;
		if(obj->IsSphere()){
			SceneSphere *sphere = (SceneSphere *) obj;
			Vector center = sphere->position + sphere->center;
			
			float A = ray.x * ray.x + ray.y * ray.y + ray.z * ray.z;
			float B = 2 * rayOrigin.x * ray.x - 2 * ray.x * center.x + 2 * rayOrigin.y * ray.y - 2 * ray.y * center.y + 2 * rayOrigin.z * ray.z - 2 * ray.z * center.z;
			float C = rayOrigin.x * rayOrigin.x + center.x * center.x - 2 * rayOrigin.x * center.x + rayOrigin.y * rayOrigin.y + center.y * center.y - 2 * rayOrigin.y * center.y + rayOrigin.z * rayOrigin.z + center.z * center.z - 2 * rayOrigin.z * center.z - sphere->radius * sphere->radius;

			float delim = B * B - 4 * A * C;
			if( delim < 0 ){
				//intersection = Vector(1.0, 0.0, 0.0, 1.0);
				//norm = Vector(0.0, 0.0, 0.0, 0.0);
				isInt = false;
			}
			else if (delim == 0){
				float t = -B/(2 * A);
				if(tt == 0 && t > 0){
					tt= t;

					intersection = Vector(rayOrigin.x + t * ray.x, rayOrigin.y + t * ray.y, rayOrigin.z + t * ray.z, 1.0);
					norm = (intersection - sphere->center).Normalize();
					s_mat = m_Scene.GetMaterial (((SceneSphere *)obj)->material);
					isInt = true;

					objName = sphere->name;
					/*if(debug){
					printf("-------------------------------\n");
					printf("t = %1.3f\n", t);
					printf("intersection_inside = (%1.2f, %1.2f, %1.2f, %1.2f)\n", intersection.x, intersection.y, intersection.z, intersection.w);
					printf("normal = (%1.2f, %1.2f, %1.2f, %1.2f)\n", norm.x, norm.y, norm.z, norm.w);
					}*/
				}
				else if(t > 0 && t < tt){

					tt = t;
					intersection = Vector(rayOrigin.x + t * ray.x, rayOrigin.y + t * ray.y, rayOrigin.z + t * ray.z, 1.0);
					norm = (intersection - sphere->center).Normalize();
					s_mat = m_Scene.GetMaterial (((SceneSphere *)obj)->material);
					isInt = true;
					objName= sphere->name;
					/*if(debug){
					printf("-------------------------------\n");
					printf("t = %1.3f\n", t);
					printf("intersection_inside = (%1.2f, %1.2f, %1.2f, %1.2f)\n", intersection.x, intersection.y, intersection.z, intersection.w);
					printf("normal = (%1.2f, %1.2f, %1.2f, %1.2f)\n", norm.x, norm.y, norm.z, norm.w);
					}*/
				}
			}
			else{
				float t1 = (-B + sqrt(delim))/(2.0 * A);
				float t2 = (-B - sqrt(delim))/(2.0 * A);
				float t =min(t1, t2);
				if(tt == 0.0 && t > 0){
					tt = t;
					intersection = Vector(rayOrigin.x + t * ray.x, rayOrigin.y + t * ray.y, rayOrigin.z + t * ray.z, 1.0);
					norm = (intersection - sphere->center).Normalize();
					/*if(debug){
						printf("-------------------------------\n");
						printf("rayOrigin = (%1.2f, %1.2f, %1.2f, %1.2f)\n", rayOrigin.x, rayOrigin.y, rayOrigin.z, rayOrigin.w);
						printf("ray = (%1.2f, %1.2f, %1.2f, %1.2f)\n", ray.x, ray.y, ray.z, ray.w);
						printf("t = %1.3f\n", t);
						printf("intersection_inside = (%1.2f, %1.2f, %1.2f, %1.2f)\n", intersection.x, intersection.y, intersection.z, intersection.w);
						printf("normal = (%1.2f, %1.2f, %1.2f, %1.2f)\n", norm.x, norm.y, norm.z, norm.w);
					}*/
					s_mat = m_Scene.GetMaterial (((SceneSphere *)obj)->material);
					isInt = true;
					objName = sphere->name;
					/*if(debug)
						getchar();*/
				}
				else if(t > 0 && t < tt){
					tt = t;
					intersection = Vector(rayOrigin.x + t * ray.x, rayOrigin.y + t * ray.y, rayOrigin.z + t * ray.z, 1.0);
					norm = (intersection - sphere->center).Normalize();
					/*if(debug){
					printf("-------------------------------\n");
					printf("t = %1.3f\n", t);
					printf("intersection_inside = (%1.2f, %1.2f, %1.2f, %1.2f)\n", intersection.x, intersection.y, intersection.z, intersection.w);
					printf("normal = (%1.2f, %1.2f, %1.2f, %1.2f)\n", norm.x, norm.y, norm.z, norm.w);
					}*/
					s_mat = m_Scene.GetMaterial (((SceneSphere *)obj)->material);
					isInt = true;
					objName = sphere->name;
				}
			}
			
		}
		else if(obj->IsSquare()){
			SceneSquare *square = (SceneSquare *) obj;
			Vector v03 = (square->vertex[0] + square->vertex[3])/2.0;
			Vector v12 = (square->vertex[1] + square->vertex[2])/2.0;
			Vector p0 = (v03 + v12)/2.0;
			p0.w = 1.0;
			/*Min-Max Box*/
			float xMin = min(min(square->vertex[0].x, square->vertex[1].x), min(square->vertex[2].x, square->vertex[3].x));
			float xMax = max(max(square->vertex[0].x, square->vertex[1].x), max(square->vertex[2].x, square->vertex[3].x));
			float zMin = min(min(square->vertex[0].z, square->vertex[3].z), min(square->vertex[1].z, square->vertex[2].z));
			float zMax = max(max(square->vertex[0].z, square->vertex[3].z), max(square->vertex[1].z, square->vertex[2].z));
			float yMin = min(min(square->vertex[0].y, square->vertex[1].y), min(square->vertex[2].y, square->vertex[3].y));
			float yMax = max(max(square->vertex[0].y, square->vertex[1].y), max(square->vertex[2].y, square->vertex[3].y));
			//printf("xMin = %1.2f, xMax = %1.2f, zMin = %1.2f, zMax = %1.2f, yMin = %1.2f, yMax = %1.2f\n", xMin, xMax, zMin, zMax, yMin, yMax);
			//printf("p0 = (%1.2f, %1.2f, %1.2f, %1.2f)\n", p0.x, p0.y, p0.z, p0.w);
			float t = (p0-rayOrigin).Dot(square->normal[0]) / ray.Dot(square->normal[0]);
			//printf("t = %1.2f\n", t);
			Vector intsect = rayOrigin + ray * t;
			intsect.w = 1.0;
			if(intsect.x < xMin-0.0001 || intsect.x > xMax+0.0001 || intsect.y < yMin-0.0001 || intsect.y > yMax + 0.0001 || intsect.z < zMin-0.0001 || intsect.z > zMax+0.0001){
				isInt = false;
			}
			else{
				if(tt == 0 && t > 0){
					tt = t;
					s_mat = m_Scene.GetMaterial(square->material[0]);
					intersection = intsect;
					norm = square->normal[0];
					isInt = true;
					if(debug){
					printf("-------------------------------\n");
					printf("t = %1.3f\n", t);
					printf("intersection_inside = (%1.2f, %1.2f, %1.2f, %1.2f)\n", intersection.x, intersection.y, intersection.z, intersection.w);
					printf("normal = (%1.2f, %1.2f, %1.2f, %1.2f)\n", norm.x, norm.y, norm.z, norm.w);
					}
				}
				else if (t > 0 && t < tt){
					
					tt = t;
					s_mat = m_Scene.GetMaterial(square->material[0]);
					intersection = intsect;
					norm = square->normal[0];
					isInt = true;
					if(debug){
					printf("-------------------------------\n");
					printf("t = %1.3f\n", t);
					printf("intersection_inside = (%1.2f, %1.2f, %1.2f, %1.2f)\n", intersection.x, intersection.y, intersection.z, intersection.w);
					printf("normal = (%1.2f, %1.2f, %1.2f, %1.2f)\n", norm.x, norm.y, norm.z, norm.w);
					}
				}
			}
		}
		return isInt;
	}
};

#endif // RAYTRACE_H
