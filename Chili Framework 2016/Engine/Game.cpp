/****************************************************************************************** 
 *	Chili DirectX Framework Version 16.07.20											  *	
 *	Game.cpp																			  *
 *	Copyright 2016 PlanetChili.net <http://www.planetchili.net>							  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include "MainWindow.h"
#include "Game.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <Windows.h>

Game::Game( MainWindow& wnd )
	:
	wnd( wnd ),
	gfx( wnd )
{
}

void Game::Go()
{
	gfx.BeginFrame();	
	UpdateModel();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::UpdateModel()
{
}

void Game::ComposeFrame()
{
	// Create Screen Buffer
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	fElapsedTime = ft.frame_timer();

	std::wstring map;

	map += L"################################";
	map += L"#..............................#";
	map += L"#..............................#";
	map += L"#...........#..................#";
	map += L"#..............................#";
	map += L"#...........#..................#";
	map += L"#..............................#";
	map += L"#..............................#";
	map += L"#...#..........................#";
	map += L"#..............................#";
	map += L"#...#..........................#";
	map += L"#..............................#";
	map += L"#.......#######................#";
	map += L"#..............................#";
	map += L"#..............................#";
	map += L"#..............................#";
	map += L"#..............................#";
	map += L"#..............................#";
	map += L"#..............................#";
	map += L"#.......X..........######......#";
	map += L"#..................#....#......#";
	map += L"#..................#....#......#";
	map += L"#..................#....#......#";
	map += L"#..................#....#......#";
	map += L"#..................#....#......#";
	map += L"#..................#....#......#";
	map += L"#..................#....#......#";
	map += L"#..................######......#";
	map += L"#..............................#";
	map += L"#..............................#";
	map += L"#..............................#";
	map += L"################################";

	// Controls
	// Handle CCW Rotation
	if (wnd.kbd.KeyIsPressed('A'))
		fPlayerA -= (50000.0f) * fElapsedTime;

	if (wnd.kbd.KeyIsPressed('D'))
		fPlayerA += (50000.0f) * fElapsedTime;

	if (wnd.kbd.KeyIsPressed('W'))
	{
		fPlayerX += sinf(fPlayerA) * 500000.0f * fElapsedTime;
		fPlayerY += cosf(fPlayerA) * 500000.0f * fElapsedTime;
	
		if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
		{
			fPlayerX -= sinf(fPlayerA) * 500000.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 500000.0f * fElapsedTime;
		}
	}

	if (wnd.kbd.KeyIsPressed('S'))
	{
		fPlayerX -= sinf(fPlayerA) * 500000.0f * fElapsedTime;
		fPlayerY -= cosf(fPlayerA) * 500000.0f * fElapsedTime;
	
		if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
		{
			fPlayerX += sinf(fPlayerA) * 500000.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 500000.0f * fElapsedTime;
		}
	}

	for (int x = 0; x < nScreenWidth; x++)
	{
		// For each column, calculate the projected ray angle into world space
		float	fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

		float	fDistanceToWall = 0;
		bool	bHitWall = false;
		bool	bBoundary = false;
		bool	bCross = false;

		float	fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
		float	fEyeY = cosf(fRayAngle);

		while (!bHitWall && fDistanceToWall < fDepth)
		{
			fDistanceToWall += 0.05f;

			int	nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
			int	nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

			// Test if ray is out of bounds
			if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
			{
				bHitWall = true;			// Just set distance to maximum depth
				fDistanceToWall = fDepth;
			}
			else
			{
				// Ray is inbounds so test to see if the ray cell is a wall block
				if (map[nTestY * nMapWidth + nTestX] == '#')
				{
					bHitWall = true;

					std::vector<std::pair<float, float>> p; // distance, dot

					for (int tx = 0; tx < 2; tx++)
						for (int ty = 0; ty < 2; ty++)
						{
							float vy = (float)nTestY + ty - fPlayerY;
							float vx = (float)nTestX + tx - fPlayerX;
							float d = sqrt(vx * vx + vy * vy);
							float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
							p.push_back(std::make_pair(d, dot));
						}

					// Sort Pairs from closest to furthest
					sort(p.begin(), p.end(), [](const std::pair<float, float> &left, const std::pair<float, float> &right) { return left.first < right.first; });

					float fBound = 0.0005f;
					if (acos(p.at(0).second) < fBound) bBoundary = true;
					if (acos(p.at(1).second) < fBound) bBoundary = true;
					//if (acos(p.at(2).second) < fBound) bBoundary = true;
				}
				else if (map[nTestY * nMapWidth + nTestX] == 'X')
				{
					bCross = true;
				}
			}
		}

		// Calculate distance to ceiling and floor
		int	nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
		int	nFloor = nScreenHeight - nCeiling;

		//if (bBoundary)		wShade = 200; // Black it out

		for (int y = 0; y < nScreenHeight; y++)
		{
			if (y < nCeiling)
			{
				// Shade ceiling based on distance
				float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
				int c = (int)(fShade * b);

				//Draw only if rgb value is 0 or higher
				if (c >= 0)
					gfx.PutPixel(x, y, c, c, c);
			}
			else if (y > nCeiling && y <= nFloor)
			{
				// Shade walls
				int wc = (int)(wShade * (fDepth - fDistanceToWall) / 20.0f);
				// Don't draw boundaries
				if (bBoundary)
					;
				//Draw only if rgb value is 0 or higher
				else if (wc >= 0)
					gfx.PutPixel(x, y, wc, wc, wc);
			}
			else if (y > nFloor)
			{
				// Shade floor based on distance
				float b = 0.0f + (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
				int R = (int)(193.0f * b);
				int G = (int)(154.0f * b);
				int B = (int)(107.0f * b);

				//Draw only if rgb value is 0 or higher
				if (B >= 0 && G >= 0 && R >= 0)
					gfx.PutPixel(x, y, R, G, B);
			}
		}
	}

	// Display Stats
	//swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

	// Display Map
	for (int nx = 0; nx < nMapWidth; nx++)
		for (int ny = 0; ny < nMapHeight; ny++)
		{
			auto draw_map = [](Graphics& gfx, int posx, int posy) mutable
			{
				for (int y = 0; y < 5; y++)
					for (int x = 0; x < 5; x++)
					{
						gfx.PutPixel(5 * posx + x, 5 * posy + y, 0, 200, 0);
					}
			};
			if (map[ny * nMapWidth + nx] == '#')
				draw_map(gfx, nx, ny);
		}
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
		{
			gfx.PutPixel(x + fPlayerX * 5, y + fPlayerY * 5, 0, 0, 255);
		}

	screen[nScreenWidth * nScreenHeight - 1] = '\0';
	//WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	delete screen;
}
