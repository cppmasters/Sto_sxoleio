#pragma once
#define _USE_MATH_DEFINES
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <vector>
#include <windows.h>
#include <fstream>
struct vec2GL
{
    float x = 0;
    float y = 0;
};
struct vec3GL
{
    float x = 0;
    float y = 0;
    float z = 0;
    vec3GL operator-(const vec3GL &other)
    {
        return vec3GL{x - other.x, y - other.y, z - other.z};
    }
    vec3GL operator+(const vec3GL &other)
    {
        return vec3GL{x + other.x, y + other.y, z + other.z};
    }
    vec3GL operator*(const vec3GL &other)
    {
        return vec3GL{x * other.x, y * other.y, z * other.z};
    }
    vec3GL operator/(float &other)
    {
        return vec3GL{x / other, y / other, z / other};
    }
};
inline float dot(const vec3GL &lhs, const vec3GL &rhs)
{
    return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}
inline vec3GL cross(const vec3GL &a, const vec3GL &b)
{
    return vec3GL{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}
float magnitude(vec3GL vec)
{
    return sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}
vec3GL normalize(vec3GL vec)
{
    float mag = magnitude(vec);
    if (mag == 0)
        return {0, 0, 0};
    return vec / mag;
}

vec2GL normalize(vec2GL vec)
{
    float mag = sqrtf((vec.x * vec.x) + (vec.y * vec.y));
    if (mag == 0)
        return {0, 0};
    return {vec.x / mag, vec.y / mag};
}
struct intVec2GL
{
    int x = 0;
    int y = 0;
};
vec3GL rotatePoint3d(vec3GL point, vec3GL angle)
{
    const float pi2rad = M_PI / 180.0f;
    const float cosAx = cosf(angle.x * pi2rad);
    const float sinAx = sinf(angle.x * pi2rad);
    const float cosAy = cosf(angle.y * pi2rad);
    const float sinAy = sinf(angle.y * pi2rad);
    const float cosAz = cosf(angle.z * pi2rad);
    const float sinAz = sinf(angle.z * pi2rad);
    float temp = 0;

    temp = point.y;
    point.y = temp * cosAx - point.z * sinAx;
    point.z = temp * sinAx + point.z * cosAx;

    temp = point.x;
    point.x = temp * cosAy + point.z * sinAy;
    point.z = -temp * sinAy + point.z * cosAy;

    temp = point.x;
    point.x = temp * cosAz - point.y * sinAz;
    point.y = temp * sinAz + point.y * cosAz;
    return point;
}
struct camera
{
    vec3GL position;
    vec3GL rotation;
    float farplane = 100.0f;
    float nearplane = 0.001f;
    float fov = 90.0f;
};
struct vertexDataContainer
{
    float *data;
    unsigned int size;
    camera cam;
};
std::vector<float> readDataSTL(std::string pathToFile)
{
    std::vector<float> imported;
    std::ifstream myFile(pathToFile);
    std::string line;
    std::string num;
    while (std::getline(myFile, line))
    {
        if (line.empty() || line[2] != 'v' || line[3] != 'e')
        {
            continue;
        }
        line.erase(0, 9);
        for (int i = 0; i < line.size(); i++)
        {
            if (line[i] != ' ')
            {
                num.push_back(line[i]);
            }
            else
            {
                if (!num.empty())
                {
                    imported.push_back(std::stof(num));
                    num.clear();
                }
            }
        }
        if (!num.empty())
        {
            imported.push_back(std::stof(num));
            num.clear();
        }
    }
    return imported;
}

class vertexTransformer
{
private:
    bool is3d;
    int vertSize;

    // Reduced memory - only essential arrays
    std::vector<float> defaultVertices;
    std::vector<float> worldVertices;
    std::vector<float> transformedVertices;

    // Cached transform state
    vec3GL currentRotation;
    vec3GL currentPosition;
    bool transformDirty;

    camera cam;

    // Pre-computed rotation matrix (stored as 9 floats)
    float rotMatrix[9];

    // Build rotation matrix from euler angles
    void buildRotationMatrix(vec3GL angle)
    {
        // Convert degrees to radians
        const float pi2rad = M_PI / 180.0f;
        float angleX = angle.x * pi2rad;
        float angleY = angle.y * pi2rad;
        float angleZ = angle.z * pi2rad;

        float cx = cosf(angleX);
        float sx = sinf(angleX);
        float cy = cosf(angleY);
        float sy = sinf(angleY);
        float cz = cosf(angleZ);
        float sz = sinf(angleZ);

        if (is3d)
        {
            // Combined rotation matrix (ZYX order)
            rotMatrix[0] = cy * cz;
            rotMatrix[1] = -cy * sz;
            rotMatrix[2] = sy;
            rotMatrix[3] = sx * sy * cz + cx * sz;
            rotMatrix[4] = -sx * sy * sz + cx * cz;
            rotMatrix[5] = -sx * cy;
            rotMatrix[6] = -cx * sy * cz + sx * sz;
            rotMatrix[7] = cx * sy * sz + sx * cz;
            rotMatrix[8] = cx * cy;
        }
        else
        {
            // 2D rotation (Z only)
            rotMatrix[0] = cz;
            rotMatrix[1] = -sz;
            rotMatrix[2] = 0;
            rotMatrix[3] = sz;
            rotMatrix[4] = cz;
            rotMatrix[5] = 0;
            rotMatrix[6] = 0;
            rotMatrix[7] = 0;
            rotMatrix[8] = 1;
        }
    }

    // Apply rotation matrix to a point
    inline vec3GL applyRotation(vec3GL v)
    {
        return vec3GL{
            rotMatrix[0] * v.x + rotMatrix[1] * v.y + rotMatrix[2] * v.z,
            rotMatrix[3] * v.x + rotMatrix[4] * v.y + rotMatrix[5] * v.z,
            rotMatrix[6] * v.x + rotMatrix[7] * v.y + rotMatrix[8] * v.z};
    }

public:
    vertexTransformer(std::vector<float> vertices, bool isMode3d)
        : is3d(isMode3d), vertSize(vertices.size()), currentRotation{0, 0, 0}, currentPosition{0, 0, 0}, transformDirty(true)
    {
        defaultVertices.reserve(vertSize);
        transformedVertices.resize(vertSize);
        worldVertices.resize(vertSize);
        defaultVertices = vertices;

        // Initialize rotation matrix to identity
        for (int i = 0; i < 9; i++)
            rotMatrix[i] = 0;
        rotMatrix[0] = rotMatrix[4] = rotMatrix[8] = 1;
    }
    void setWorldSpace()
    {
        float *world = worldVertices.data();
        const float *defaults = defaultVertices.data();

        if (is3d)
        {
            for (int i = 0; i < vertSize; i += 3)
            {
                vec3GL rotated = applyRotation({defaults[i], defaults[i + 1], defaults[i + 2]});
                world[i] = rotated.x + currentPosition.x;
                world[i + 1] = rotated.y + currentPosition.y;
                world[i + 2] = rotated.z + currentPosition.z;
            }
        }
        else
        {
            for (int i = 0; i < vertSize; i += 2)
            {
                vec3GL rotated = applyRotation({defaults[i], defaults[i + 1], 0});
                world[i] = rotated.x + currentPosition.x;
                world[i + 1] = rotated.y + currentPosition.y;
            }
        }
    }

    vertexDataContainer unpackWorld()
    {
        vertexDataContainer d;
        d.cam = cam;
        d.data = worldVertices.data();
        d.size = vertSize;
        return d;
    }
    void setRotation(vec3GL angle)
    {
        currentRotation = angle;
        transformDirty = true;
        buildRotationMatrix(angle);
    }

    void setPosition(vec3GL position)
    {
        currentPosition = position;
        transformDirty = true;
    }

    void setViewSpace(camera camNow)
    {
        cam = camNow;

        // Build camera rotation matrix
        float camRotMatrix[9];
        float cx = cosf(-cam.rotation.x);
        float sx = sinf(-cam.rotation.x);
        float cy = cosf(-cam.rotation.y);
        float sy = sinf(-cam.rotation.y);
        float cz = cosf(-cam.rotation.z);
        float sz = sinf(-cam.rotation.z);

        if (is3d)
        {
            camRotMatrix[0] = cy * cz;
            camRotMatrix[1] = -cy * sz;
            camRotMatrix[2] = sy;
            camRotMatrix[3] = sx * sy * cz + cx * sz;
            camRotMatrix[4] = -sx * sy * sz + cx * cz;
            camRotMatrix[5] = -sx * cy;
            camRotMatrix[6] = -cx * sy * cz + sx * sz;
            camRotMatrix[7] = cx * sy * sz + sx * cz;
            camRotMatrix[8] = cx * cy;
        }
        else
        {
            camRotMatrix[0] = cz;
            camRotMatrix[1] = -sz;
            camRotMatrix[3] = sz;
            camRotMatrix[4] = cz;
        }

        // Direct pointer access for speed
        float *transformed = transformedVertices.data();
        const float *defaults = defaultVertices.data();

        if (is3d)
        {
            // Process all vertices in one pass: rotate -> translate -> view transform
            for (int i = 0; i < vertSize; i += 3)
            {
                // Apply object rotation
                vec3GL rotated = applyRotation({defaults[i], defaults[i + 1], defaults[i + 2]});

                // Apply object position
                float worldX = rotated.x + currentPosition.x;
                float worldY = rotated.y + currentPosition.y;
                float worldZ = rotated.z + currentPosition.z;

                // Translate to camera space
                float relX = worldX - cam.position.x;
                float relY = worldY - cam.position.y;
                float relZ = worldZ - cam.position.z;

                // Apply camera rotation
                transformed[i] = camRotMatrix[0] * relX + camRotMatrix[1] * relY + camRotMatrix[2] * relZ;
                transformed[i + 1] = camRotMatrix[3] * relX + camRotMatrix[4] * relY + camRotMatrix[5] * relZ;
                transformed[i + 2] = camRotMatrix[6] * relX + camRotMatrix[7] * relY + camRotMatrix[8] * relZ;
            }
        }
        else
        {
            // 2D path
            for (int i = 0; i < vertSize; i += 2)
            {
                // Apply object rotation
                vec3GL rotated = applyRotation({defaults[i], defaults[i + 1], 0});

                // Apply object position
                float worldX = rotated.x + currentPosition.x;
                float worldY = rotated.y + currentPosition.y;

                // Translate to camera space
                float relX = worldX - cam.position.x;
                float relY = worldY - cam.position.y;

                // Apply camera rotation
                transformed[i] = camRotMatrix[0] * relX + camRotMatrix[1] * relY;
                transformed[i + 1] = camRotMatrix[3] * relX + camRotMatrix[4] * relY;
            }
        }
    }

    void project()
    {
        if (is3d)
        {
            const float pi2rad = M_PI / 180.0f;
            float fov = std::clamp(cam.fov, 1.0f, 130.0f);
            fov = fov * pi2rad;
            float scale = 1.0f / tanf(fov * 0.5f);

            float *transformed = transformedVertices.data();

            for (int i = 0; i < vertSize; i += 3)
            {
                float z = transformed[i + 2];

                if (z < cam.nearplane || z > cam.farplane)
                    continue;

                float invZ = 1.0f / z;
                transformed[i] *= scale * invZ;
                transformed[i + 1] *= scale * invZ;
            }
        }
    }

    vertexDataContainer unpack()
    {
        vertexDataContainer d;
        d.cam = cam;
        d.data = transformedVertices.data();
        d.size = vertSize;
        return d;
    }

    camera cameraData()
    {
        return cam;
    }

    int size()
    {
        return vertSize;
    }

    ~vertexTransformer() {}
};
class asciiGL
{
private:
    void *operator new(size_t size) = delete;
    void operator delete(void *ptr) = delete;
    void *operator new[](size_t size) = delete;
    void operator delete[](void *ptr) = delete;
    camera cam;
    inline float getSide(intVec2GL a, intVec2GL b, intVec2GL p)
    {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    }
    bool is3d;
    int resX;
    int resY;
    float aspectRatio;
    char backroundChar;
    int screenBufferSize;
    std::vector<char> screenBuffer;
    inline static bool boundsCheck(vec2GL cornerA, vec2GL cornerB, vec2GL p)
    {
        bool status = false;
        vec2GL largeCords = cornerA;
        vec2GL smallCords = cornerB;
        if (cornerA.x < cornerB.x)
        {
            smallCords.x = cornerA.x;
            largeCords.x = cornerB.x;
        }
        if (cornerA.y < cornerB.y)
        {
            smallCords.y = cornerA.y;
            largeCords.y = cornerB.y;
        }
        if (p.x >= smallCords.x && p.x <= largeCords.x && p.y >= smallCords.y && p.y <= largeCords.y)
        {
            status = true;
        }
        return status;
    }
    inline void drawPointRaw(intVec2GL rawPos, char drawChar)
    {
        int x = roundf(rawPos.x);
        int y = roundf(rawPos.y);
        int location = x * 2 + (resX * 2 * y);
        if (x < resX && x >= 0 && y >= 0 && y < resY && screenBuffer[location] == ' ')
            screenBuffer[location] = drawChar;
    }
    inline intVec2GL normToRaw(vec2GL normalizedPoint)
    {
        int x = roundf((normalizedPoint.x / (aspectRatio) + 1.0f) * (resX / 2.0f));
        int y = roundf((-normalizedPoint.y + 1.0f) * (resY / 2.0f));
        return {x, y};
    }

public:
    void init(unsigned int resolutionX, unsigned int resolutionY, bool isMode3d)
    {
        is3d = isMode3d;
        resX = resolutionX;
        resY = resolutionY;
        aspectRatio = (float)resX / (float)resY;
        std::ios_base::sync_with_stdio(false);
        std::cin.tie(NULL);
        screenBufferSize = resX * resY * 2;
        screenBuffer.resize(screenBufferSize);
        for (int i = 0; i < screenBufferSize; i++)
        {
            screenBuffer[i] = '.';
        }
        fillScreen(' ');
    }
    char brightnessChar(float normalizedBrightnessValue)
    {
        if (normalizedBrightnessValue > 1.0f)
            normalizedBrightnessValue = 1.0f;
        if (normalizedBrightnessValue < 0.0f)
            normalizedBrightnessValue = 0.0f;
        static const char brightnessTable[] = R"(.-"<l{Jo25&g#Q@)";
        int index = std::roundf(14 * normalizedBrightnessValue);
        return brightnessTable[index];
    }
    inline void hideCursor(bool hideCursor)
    {
        if (hideCursor)
        {
            std::cout << "\x1b[?25l";
        }
        else
        {
            std::cout << "\x1b[?25h";
        }
    }
    void fillScreen(char fillChar)
    {
        backroundChar = fillChar;
        int offset = 1;
        for (int i = 0; i < screenBufferSize; i++)
        {
            if (i % 2 == 0)
            {
                screenBuffer[i] = fillChar;
            }
            else
            {
                if (i != (resX * 2 * offset) - 1)
                    screenBuffer[i] = ' ';
                else
                {
                    screenBuffer[i] = '\n';
                    offset++;
                }
            }
        }
    }
    void displayBuffer()
    {
        std::cout << "\x1b[H";
        std::cout.write(screenBuffer.data(), screenBufferSize);
        std::cout.flush();
    }

    inline void drawPoint(vec2GL normPos, char drawChar)
    {
        intVec2GL pixel = normToRaw(normPos);
        int location = pixel.x * 2 + (resX * 2 * pixel.y);

        if (pixel.x < resX && pixel.x >= 0 && pixel.y >= 0 && pixel.y < resY)
        {
            screenBuffer[location] = drawChar;
        }
    }
    void drawPoints(vertexDataContainer data, char drawChar)
    {
        int step;
        if (is3d)
            step = 3;
        else
            step = 2;
        for (int i = 0; i < data.size; i = i + step)
        {
            if (data.data[i + 2] > data.cam.nearplane || !is3d)
                drawPoint({data.data[i], data.data[i + 1]}, drawChar);
        }
    }
    void drawLine(vec2GL pointANorm, vec2GL pointBNorm, char drawChar)
    {
        // 2d clipping
        std::vector<vec2GL> validPoints;
        float vLineA = -1 * aspectRatio;
        float vLineB = 1 * aspectRatio;

        if (boundsCheck({vLineA, -1}, {vLineB, 1}, pointANorm))
            validPoints.push_back(pointANorm);
        if (boundsCheck({vLineA, -1}, {vLineB, 1}, pointBNorm))
            validPoints.push_back(pointBNorm);

        float slope = 0.0f;
        bool validSlope = true;
        if (pointANorm.x - pointBNorm.x != 0 && pointANorm.y - pointBNorm.y != 0)
        {
            slope = (pointANorm.y - pointBNorm.y) / (pointANorm.x - pointBNorm.x);
        }
        else
        {
            validSlope = false;
        }
        float y = 0.0f;
        float x = 0.0f;
        float lines[4] = {vLineA, vLineB, 1, -1};

        if (validSlope && validPoints.size() < 2)
        {

            for (int i = 0; i < 4; i++)
            {
                if (i < 2) // check the verticals
                {
                    x = lines[i];
                    y = slope * (x - pointANorm.x) + pointANorm.y;
                    if (y >= -1 && y <= 1)
                    {
                        if (boundsCheck(pointANorm, pointBNorm, {x, y}))
                        {
                            validPoints.push_back({x, y});
                        }
                    }
                }
                else // check the horizontals
                {
                    y = lines[i];
                    x = (y - pointANorm.y) / slope + pointANorm.x;
                    if (x >= vLineA && x <= vLineB)
                    {
                        if (boundsCheck(pointANorm, pointBNorm, {x, y}))
                        {
                            validPoints.push_back({x, y});
                        }
                    }
                }
            }
        }
        else
        {
            if (pointANorm.x - pointBNorm.x == 0)
            {
                y = 1;
                x = pointANorm.x;
                if (x >= vLineA && x <= vLineB)
                {
                    if (boundsCheck(pointANorm, pointBNorm, {x, y}))
                    {
                        validPoints.push_back({x, y});
                    }
                }

                y = -1;
                x = pointANorm.x;
                if (x >= vLineA && x <= vLineB)
                {
                    if (boundsCheck(pointANorm, pointBNorm, {x, y}))
                    {
                        validPoints.push_back({x, y});
                    }
                }
            }
            if (pointANorm.y - pointBNorm.y == 0)
            {
                x = vLineA;
                y = pointANorm.y;
                if (y >= -1 && y <= 1)
                {
                    if (boundsCheck(pointANorm, pointBNorm, {x, y}))
                    {
                        validPoints.push_back({x, y});
                    }
                }

                x = vLineB;
                if (y >= -1 && y <= 1)
                {
                    if (boundsCheck(pointANorm, pointBNorm, {x, y}))
                    {
                        validPoints.push_back({x, y});
                    }
                }
            }
        }

        // draw line
        if (validPoints.size() >= 2)
        {
            intVec2GL stepPoint = normToRaw(validPoints[0]);
            intVec2GL staticPoint = normToRaw(validPoints[1]);

            int8_t leaderStep = -1;
            int8_t minorStep = -1;
            uint16_t error = 0;
            int dy = stepPoint.y - staticPoint.y;
            int dx = stepPoint.x - staticPoint.x;
            bool xLeader = false;
            if (dx < 0)
            {
                leaderStep = 1;
                dx = -dx;
            }
            if (dy < 0)
            {
                minorStep = 1;
                dy = -dy;
            }
            if (dx >= dy)
                xLeader = true;

            // drawPointRaw(stepPoint, drawChar);
            if (xLeader)
            {
                while (stepPoint.x != staticPoint.x || stepPoint.y != staticPoint.y)
                {
                    stepPoint.x = stepPoint.x + leaderStep;
                    error = error + dy;
                    if (error >= dx)
                    {
                        error = error - dx;
                        stepPoint.y = stepPoint.y + minorStep;
                    }
                    drawPointRaw(stepPoint, drawChar);
                }
            }
            else
            {
                while (stepPoint.x != staticPoint.x || stepPoint.y != staticPoint.y)
                {
                    stepPoint.y = stepPoint.y + minorStep;
                    error = error + dx;
                    if (error >= dy)
                    {
                        error = error - dy;
                        stepPoint.x = stepPoint.x + leaderStep;
                    }
                    drawPointRaw(stepPoint, drawChar);
                }
            }
        }
        // drawPoint({validPoints[0].x*0.90f,validPoints[0].y*0.90f},'&');
        // drawPoint({validPoints[1].x*0.90f,validPoints[1].y*0.90f},'&');
    }
    void drawLines(vertexDataContainer d, char drawChar)
    {
        if (is3d)
        {
            for (int i = 0; i < d.size; i = i + 6)
            {
                if (d.data[i + 2] > d.cam.nearplane && d.data[i + 5] > d.cam.nearplane)
                {
                    drawLine({d.data[i], d.data[i + 1]}, {d.data[i + 3], d.data[i + 4]}, drawChar);
                }
            }
        }
        else
        {
            for (int i = 0; i < d.size; i = i + 4)
            {
                drawLine({d.data[i], d.data[i + 1]}, {d.data[i + 2], d.data[i + 3]}, drawChar);
            }
        }
    }
    void drawTriangle(vec2GL a, vec2GL b, vec2GL c, char drawChar)
    {

        intVec2GL rawA = normToRaw(a);
        intVec2GL rawB = normToRaw(b);
        intVec2GL rawC = normToRaw(c);
        intVec2GL list[3] = {rawA, rawB, rawC};
        intVec2GL rawMax = list[0];
        intVec2GL rawMin = list[0];
        for (int i = 0; i < 3; i++)
        {
            if (list[i].x > rawMax.x)
                rawMax.x = list[i].x;
            if (list[i].y > rawMax.y)
                rawMax.y = list[i].y;
            if (list[i].x < rawMin.x)
                rawMin.x = list[i].x;
            if (list[i].y < rawMin.y)
                rawMin.y = list[i].y;
        }
        rawMax = {rawMax.x, rawMax.y};
        rawMin = {rawMin.x, rawMin.y};
        int startY = std::max(0, std::min(rawMax.y, rawMin.y));
        int endY = std::min(resY - 1, std::max(rawMax.y, rawMin.y));
        int startX = std::max(0, std::min(rawMin.x, rawMax.x));
        int endX = std::min(resX - 1, std::max(rawMin.x, rawMax.x));
        intVec2GL scan;
        float sideA;
        float sideB;
        float sideC;
        for (int y = startY; y <= endY; y++)
        {
            for (int x = startX; x <= endX; x++)
            {
                scan = {x, y};

                sideA = getSide(rawA, rawB, scan);
                sideB = getSide(rawB, rawC, scan);
                sideC = getSide(rawC, rawA, scan);
                if ((sideA >= 0 && sideB >= 0 && sideC >= 0) || (sideA <= 0 && sideB <= 0 && sideC <= 0))
                {
                    drawPointRaw(scan, drawChar);
                }
            }
        }
    }
    void drawTriangleClip(vec3GL a, vec3GL b, vec3GL c, char drawChar, camera camNow)
    {
        if (a.z > camNow.nearplane && b.z > camNow.nearplane && c.z > camNow.nearplane)
        {
            drawTriangle({a.x, a.y}, {b.x, b.y}, {c.x, c.y}, drawChar);
        }
    }
    void drawTriangles(vertexDataContainer data, char drawChar)
    {
        if (is3d)
        {
            if (data.size < 8)
                return;
            for (int i = 0; i < data.size; i = i + 9)
            {
                if (data.data[i + 2] > data.cam.nearplane && data.data[i + 5] > data.cam.nearplane && data.data[i + 8] > data.cam.nearplane)
                {
                    drawTriangle({data.data[i], data.data[i + 1]}, {data.data[i + 3], data.data[i + 4]}, {data.data[i + 6], data.data[i + 7]}, drawChar);
                }
            }
        }
        else
        {
            if (data.size < 6)
            {
                return;
            }
            for (int i = 0; i < data.size; i = i + 6)
            {
                drawTriangle({data.data[i], data.data[i + 1]}, {data.data[i + 2], data.data[i + 3]}, {data.data[i + 4], data.data[i + 5]}, drawChar);
            }
        }
    }
    ~asciiGL()
    {
        std::cout << "runing destructor" << std::endl;
    }
};