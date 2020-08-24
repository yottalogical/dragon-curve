#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <utility>
#include <functional>
#include <unordered_map>
#include <cstdlib>

class FileException {
public:
	FileException(bool input_file, const std::string& filename) : input_file(input_file), filename(filename) {}
	
	friend std::ostream& operator<<(std::ostream& out, const FileException& exception);
	
private:
	bool input_file;
	std::string filename;
};

std::ostream& operator<<(std::ostream& out, const FileException& exception) {
	return out << "Unable to " << (exception.input_file ? "open" : "save" ) << " `" << exception.filename << '`';
}

class Dragon {
public:
	struct Pixel {
		unsigned short int red = 0, green = 0, blue = 0;
	};
	
	Dragon(size_t iterations, const std::string& tile_file, Pixel color) : tile(tile_file), color(color) {
		const Curve curve(iterations);
		const Coordinates starting_position = set_dimensions(curve);
		draw(curve, starting_position);
	}
	
	void save(const std::string& filename) {
		std::ofstream file(filename);
		if (!file.is_open()) throw FileException(false, filename);
		
		file << "P3\n";
		file << canvas.front().size() << ' ' << canvas.size() << '\n';
		file << 255 << '\n';
		
		for (const std::vector<bool>& row : canvas) {
			for (bool on : row) {
				if (on) {
					file << color.red << ' ';
					file << color.green << ' ';
					file << color.blue << ' ';
				} else {
					file << 0 << ' ';
					file << 0 << ' ';
					file << 0 << ' ';
				}
			}
			
			file << '\n';
		}
	}
	
private:
	class Curve {
	public:
		Curve(size_t iterations) {
			for (size_t i = 0; i < iterations; ++i) {
				std::vector<bool> to_append = turns;
				to_append.flip();
				
				turns.push_back(true);
				reverse_copy(to_append.begin(), to_append.end(), back_inserter(turns));
			}
		}
		
		auto begin() const {
			return turns.begin();
		}
		
		auto end() const {
			return turns.end();
		}
		
	private:
		std::vector<bool> turns;
	};
	
	struct Coordinates {
		int x = 0, y = 0;
	};
	
	enum class Direction {north, east, south, west};
	
	static Direction reverse(Direction direction) {
		switch (direction) {
			case Direction::north:
				return Direction::south;
			case Direction::east:
				return Direction::west;
			case Direction::south:
				return Direction::north;
			case Direction::west:
				return Direction::east;
		}
	}
	
	static void rotate(Direction& direction, bool turn) {
		if (turn) {
			switch (direction) {
				case Direction::north:
					direction = Direction::east;
					break;
				case Direction::east:
					direction = Direction::south;
					break;
				case Direction::south:
					direction = Direction::west;
					break;
				case Direction::west:
					direction = Direction::north;
					break;
			}
		} else {
			switch (direction) {
				case Direction::north:
					direction = Direction::west;
					break;
				case Direction::east:
					direction = Direction::north;
					break;
				case Direction::south:
					direction = Direction::east;
					break;
				case Direction::west:
					direction = Direction::south;
					break;
			}
		}
	}
	
	static void translate(Coordinates& position, Direction direction) {
		switch (direction) {
			case Direction::north:
				--position.y;
				break;
			case Direction::east:
				++position.x;
				break;
			case Direction::south:
				++position.y;
				break;
			case Direction::west:
				--position.x;
				break;
		}
	}
	
	class Tile {
	public:
		Tile(const std::string& filename) {
			std::ifstream file(filename);
			if (!file.is_open()) throw FileException(true, filename);
			
			size_t size;
			file >> size;
			std::vector<std::vector<bool>> north_west_grid(size, std::vector<bool>(size));
			
			for (size_t i = 0; i < north_west_grid.size(); ++i) {
				for (size_t j = 0; j < north_west_grid.front().size(); ++j) {
					bool on;
					file >> on;
					north_west_grid[i][j] = on;
				}
			}
			
			grids[{Direction::north, Direction::west}] = std::move(north_west_grid);
			grids[{Direction::north, Direction::east}] = rotate(grids.at({Direction::north, Direction::west}));
			grids[{Direction::south, Direction::east}] = rotate(grids.at({Direction::north, Direction::east}));
			grids[{Direction::south, Direction::west}] = rotate(grids.at({Direction::south, Direction::east}));
		}
		
		void draw(std::vector<std::vector<bool>>& canvas, Coordinates position, Direction from, Direction to) const {
			const std::vector<std::vector<bool>>& grid = grids.at({from, to});
			
			for (size_t i = 0; i < grid.size(); ++i) {
				for (size_t j = 0; j < grid.front().size(); ++j) {
					if (grid[i][j]) {
						canvas[position.y * grid.size() + i][position.x * grid.front().size() + j] = true;
					}
				}
			}
		}
		
		size_t size() const {
			return grids.at({Direction::north, Direction::west}).size();
		}
		
	private:
		struct Corner {
			Direction from, to;
			
			struct Hash {
				size_t operator()(Corner corner) const {
					static constexpr std::hash<Direction> hasher;
					return hasher(corner.from) + hasher(corner.to);
				}
			};
			
			bool operator==(Corner other) const {
				if (from == other.from && to == other.to) return true;
				if (from == other.to && to == other.from) return true;
				return false;
			}
		};
		
		std::unordered_map<Corner, std::vector<std::vector<bool>>, Corner::Hash> grids;
		
		static std::vector<std::vector<bool>> rotate(const std::vector<std::vector<bool>>& grid) {
			std::vector<std::vector<bool>> new_tile(grid.front().size(), std::vector<bool>(grid.size()));
			
			for (size_t i = 0; i < grid.size(); ++i) {
				for (size_t j = 0; j < grid.front().size(); ++j) {
					new_tile[j][grid.size() - i - 1] = grid[i][j];
				}
			}
			
			return new_tile;
		}
	};
	
	Tile tile;
	Pixel color;
	std::vector<std::vector<bool>> canvas;
	
	Coordinates set_dimensions(const Curve& curve) {
		Direction pointing = Direction::north;
		Coordinates position;
		Coordinates minimum;
		Coordinates maximum;
		
		for (bool turn : curve) {
			rotate(pointing, turn);
			translate(position, pointing);
			
			minimum.x = std::min(position.x, minimum.x);
			minimum.y = std::min(position.y, minimum.y);
			maximum.x = std::max(position.x, maximum.x);
			maximum.y = std::max(position.y, maximum.y);
		}
		
		canvas.resize((maximum.y - minimum.y + 1) * tile.size(), std::vector<bool>((maximum.x - minimum.x + 1) * tile.size(), false));
		return {-minimum.x, -minimum.y};
	}
	
	void draw(const Curve& curve, Coordinates starting_position) {
		Direction pointing = Direction::north;
		Coordinates position = starting_position;
		
		for (bool turn : curve) {
			const Direction before = pointing;
			rotate(pointing, turn);
			
			tile.draw(canvas, position, reverse(before), pointing);
			
			translate(position, pointing);
		}
	}
};

void print_help(const std::string& executable) {
	std::cout << "Useage:\n";
	std::cout << executable << " [TILE INPUT FILE] [OUTPUT FILE] [ITERATIONS]\n";
	std::cout << executable << " [TILE INPUT FILE] [OUTPUT FILE] [ITERATIONS] [RED (0-255)] [GREEN (0-255)] [BLUE (0-255)]\n";
}

int main(int argc, const char** argv) {
	std::ios_base::sync_with_stdio(false);
	
	std::string input_file;
	std::string output_file;
	size_t iterations = 0;
	Dragon::Pixel color = {0, 255, 0};
	
	switch (argc) {
		case 7:
			color.red = std::min(std::max(atoi(argv[4]), 0), 255);
			color.green = std::min(std::max(atoi(argv[5]), 0), 255);
			color.blue = std::min(std::max(atoi(argv[6]), 0), 255);
		case 4:
			input_file = argv[1];
			output_file = argv[2];
			iterations = std::max(atoi(argv[3]), 0);
			break;
		default:
			print_help(argv[0]);
			return 1;
	}
	
	try {
		Dragon dragon(iterations, input_file, color);
		dragon.save(output_file);
	} catch (const FileException& exception) {
		std::cerr << exception << '\n';
		return 1;
	}
}
