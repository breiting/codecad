---@meta
---@module "ccad.util.func"

---@class Shape

local F = {}

--- Sample y = f(x) over [x0, x1] with n points (>=2).
---@param f fun(x:number):number
---@param x0 number
---@param x1 number
---@param n integer|nil
---@param keep_invalid? boolean|nil
---@return number[][]
function F.sample_xy(f, x0, x1, n, keep_invalid) end

--- Sample a parametric curve (x(t), y(t)) over [t0, t1] with n points (>=2).
---@param fx fun(t:number):number
---@param fy fun(t:number):number
---@param t0 number
---@param t1 number
---@param n integer|nil
---@return number[][]
function F.sample_param(fx, fy, t0, t1, n) end

--- Sample a polar curve r = fr(theta_rad) with theta in degrees [a0..a1].
---@param fr fun(theta_rad:number):number
---@param a0_deg number
---@param a1_deg number
---@param n integer|nil
---@return number[][]
function F.sample_polar(fr, a0_deg, a1_deg, n) end

--- Uniformly sample a circular arc from radius and chord length.
---@param R number
---@param L number
---@param N integer
---@return number[][]
function F.sample_arc(R, L, N) end

--- Build an XY planar face from a polyline; closed by default.
---@param points number[][]
---@param closed? boolean
---@return Shape
function F.face_xy(points, closed) end

--- Rectangle whose top edge follows y=f(x) (XY face).
---@param xL number
---@param xR number
---@param yBottom number
---@param yJoin number
---@param f fun(x:number):number
---@param n integer|nil
---@return Shape
function F.rect_with_func_top(xL, xR, yBottom, yJoin, f, n) end

--- Revolve a polar-defined radius r(theta) into a 360° solid.
---@param fr fun(theta_rad:number):number
---@param a0_deg number
---@param a1_deg number
---@return Shape
function F.revolve_from_polar(fr, a0_deg, a1_deg) end

return F
