# 3[D] Motor

A high-performance, real-time 3D Geometric Evaluation & Rendering Engine built in Modern C++20 and OpenGL. Inspired by parametric geometric tools such as GeoGebra 3D, this project provides an interactive environment for dynamically defining, parsing, and rendering 3D spatial primitives.

---

## 🏛️ System Architecture

The engine decouples user command processing from GPU graphics execution through a three-stage compiler frontend connected to a dual-pass rendering pipeline:

```text
 +------------------+     +-------------------+     +-------------------+     +-------------------+
 |  ImGui Console   | --> | Lexical Analyzer  | --> | Recursive Parser  | --> |   AST Evaluator   |
 |   Command Line   |     |    std::string    |     | std::vector<Node> |     |   RuntimeValue    |
 +------------------+     +-------------------+     +-------------------+     +-------------------+
                                                                                    |
                                                                                    v
 +------------------+     +-------------------+     +-------------------+     +-------------------+
 |   GPU Shader     | <-- |  Dual-Pass Queue  | <-- | getDistanceToCam  | <-- | Global Context    |
 | glDrawArrays Call|     | Opaque/Transparent|     | Primitive Distance|     | Context::object   |
 +------------------+     +-------------------+     +-------------------+     +-------------------+

```

* **Lexical Analyzer (Lexer):** Scans command strings into tokens using non-allocating `std::string_view` slices while recording column offsets for exact diagnostic tracking.
* **Recursive Descent Parser:** Constructs a contiguous Abstract Syntax Tree (AST) stored in cache-friendly arrays (`std::vector<Node>`), enforcing syntax rules and argument limits ($N \le 3$).
* **AST Evaluator:** Recursively resolves types at runtime via polymorphic `std::variant` containers (`RuntimeValue`), executing vector algebra and handling geometric degeneracies.
* **Dual-Pass Renderer:** Measures primitive distances relative to camera space, sorting objects into distinct opaque and transparent render queues to ensure blending accuracy.

---

## 🛠️ Key Engine Features

### 1. Embedded Scripting Engine & Diagnostic Feedback

* **Dynamic DSL Processing:** Evaluates complex geometric commands and variable bindings in real time.
* **Column-Exact Error Reporting:** Highlights precise character offsets in the ImGui terminal for lexical, syntactic, or semantic type errors.

### 2. Dual-Pass Transparency & OIT Mitigation

To solve depth-buffer fighting and flickering when rendering overlapping $2\text{D}$ transparent planes ($\alpha = 0.2$):

* **Opaque Pass ($\alpha = 1.0$):** Renders $0\text{D}$ points and $1\text{D}$ lines/vectors with Depth Testing enabled and `glDepthMask(GL_TRUE)`. Objects are sorted **Front-to-Back** to optimize GPU early-Z rejection.
* **Transparent Pass ($\alpha < 1.0$):** Renders transparent planes sorted strictly **Back-to-Front** relative to camera distance with depth buffer writing disabled via `glDepthMask(GL_FALSE)`.

### 3. Geometric Degeneracy & Orthogonalization

* **Collinearity Handling:** When constructing a 3D plane from three points ($\text{Plane}(A, B, C)$), the engine evaluates vector alignment via dot products. If input vectors are collinear, it falls back to Gram-Schmidt orthogonalization to construct a stable orthogonal basis and normal vector.
* **Topological Distance Metrics:** Custom distance evaluations adapt to primitive dimensionality ($0\text{D}$ point distance, $1\text{D}$ segment projection clamping, $1\text{D}$ infinite line cross-product height, and $2\text{D}$ plane mesh centroid metrics).

---

## ⌨️ Command API Directory

| Function Command | Parameter Signatures | Derived Geometric Type | Description |
| --- | --- | --- | --- |
| `Point` | `Point(x, y, z)` | `Point` ($0\text{D}$) | Creates a point literal at coordinates $(x, y, z)$. |
| `Vector` | `Vector(P)` <br> `Vector(P1, P2)` | `Vector` ($1\text{D}$) | Creates a directed vector from the origin to $P$, or spanning $P_1 \rightarrow P_2$. |
| `Segment` | `Segment(P1, P2)` | `Segment` ($1\text{D}$) | Creates a bounded segment between points $P_1$ and $P_2$. |
| `Line` | `Line(P, V)` <br> `Line(P1, P2)` | `Line` ($1\text{D}$) | Creates an infinite line passing through $P$ along direction $V$ or connecting $P_1$ and $P_2$. |
| `Plane` | `Plane(P, V)` <br> `Plane(A, B, C)` | `Plane` ($2\text{D}$) | Constructs an infinite surface from a point and normal vector, or from 3 points. |
| `Cross` | `Cross(V1, V2)` | `Vector` ($1\text{D}$) | Calculates the vector cross product $\mathbf{V}_1 \times \mathbf{V}_2$. |
| `Intersect` | `Intersect(Obj1, Obj2)` | `Point` / `Line` | Evaluates analytic intersections between Line-Line, Line-Plane, or Plane-Plane. |

---

## 📸 Media & Demos

### Main Interface

*(Screenshot placeholder: Interactive 3D Viewport with Coordinate Axes and ImGui Terminal)*

### Interactive Command Console & Real-Time Parsing

*(GIF placeholder: Executing script commands, defining variables, and live visual update)*

### Plane Intersections & Transparency Rendering

*(GIF/Screenshot placeholder: Multi-plane blending with Z-buffer stability)*

---

## 🏗️ Building the Project

*(This section will be updated with build instructions, application icons, and platform-specific configurations in an upcoming project release).*

---

## ✍️ Author's Note

Building **3[D] Motor** has been an incredible milestone in my journey as a Computer Science student aiming to specialize in graphics programming and software architecture.

Moving from foundational C++ concepts to designing a custom language interpreter and an OpenGL renderer forced me to connect mathematical theory with low-level computing. Implementing linear algebra concepts, working out spatial projections, handling transparency sorting, and building an interactive frontend with Dear ImGui presented non-trivial engineering challenges that significantly deepened my understanding of systems architecture.

There is always more to refine and build, and I am excited to continue expanding my knowledge in low-level graphics API development.

---

## 📚 Credits & References

* **LearnCPP.com:** Fundamental C++ techniques and core modern language paradigms.
* **3D Math Primer for Graphics and Game Development:** Mathematical intuition for vector algebra, projections, and spatial transformations.
* **GeoGebra 3D:** Functional inspiration for interactive parametric geometric modeling tools.
* **Dear ImGui & GLFW:** GUI framework and windowing abstraction layers.