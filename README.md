# Project: 3D Scene Visualization (CS-330)

## 🖥️ Project Overview
This repository contains a 3D scene developed using C++ and Modern OpenGL. 
The project showcases the implementation of complex 3D meshes, texture mapping, 
and a Phong lighting model (Ambient, Diffuse, and Specular components).

---

## 🧠 Reflection

### 1. How do I approach designing software?
* **New Design Skills:** Through this project, I transitioned from thinking in 2D layouts to 3D spatial environments. Crafting meshes taught me to visualize objects as a collection of vertices and indices, while texture mapping required a precise understanding of UV coordinate systems.
* **Design Process:** I followed an iterative "Shape-First" process: first establishing the geometric primitive (the mesh), then refining the camera interactivity, and finally layering on the lighting and textures to bring realism to the scene.
* **Future Application:** The "Component-Based" design used here—separating the shader logic from the main application—is a tactic I’ll carry into future full-stack development to keep code modular and scalable.

### 2. How do I approach developing programs?
* **New Development Strategies:** Using the OpenGL pipeline required a mindset shift toward "Graphics State Management." I learned to manage buffers (VBOs, VAOs) and handle memory on the GPU, which is much more hardware-conscious than standard high-level programming.
* **Iteration Factor:** Iteration was critical. The scene began as a single colored triangle and evolved through constant debugging of the vertex shaders and fragment shaders. If the lighting looked "flat," I had to iterate on the normal vectors and light positioning.
* **Code Evolution:** My approach evolved from writing monolithic code to using helper classes for Camera and Shader management. This evolution allowed for a more fluid development cycle where I could focus on aesthetics rather than boilerplate code.

### 3. How can computer science help me reach my goals?
* **Educational Pathway:** Understanding computational graphics demystifies how software interacts with hardware. This knowledge will be foundational in future courses involving UI/UX design and high-performance computing.
* **Professional Pathway:** Visualizing data and creating immersive environments are highly sought-after skills in industries ranging from simulation and training to game development and VR. These skills prove I can handle complex, math-heavy logic to solve visual problems.
