
# Splitter wedges, pole regions, and detector properties
To understand detector space split, assumptions, and limitations, see notes at the top of `SplitterNotebook.ipynb` file.


# Splitter Notebook
The `SplitterNotebook.ipynb` provides visualizations and visual checks for point-in-region check functions. Notebook defines, visualizes, and mentions limitations for wedges and pole regions specifying split of the detector space. Some of functions present there may be useful for visualizations as well as actual computation in C++/SYCL implementation.


# Test Data Generator
The `TestDataGenerator.py` script purpose is to generate data for tests of splitter implementation in C++. The script uses naive approach to determine which regions contain input points. Checking process is visualized in SplitterNotebook.ipynb. **Every change to any of related functions in `SplitterNotebook.ipynb` has to be mirrored in `TestDataGenerator/TestDataGenerator/py` and vice versa.**

## General desciption
The scripts reads **detector properties** and **splitter settings** from a JSON file, which contains about the wedges and pole regions. Reads space points from a CSV file. Assigns a list of region IDs to each point by checking if the point lies within a region. Outputs the processed space points to another CSV file, appending the identified region IDs.

## Usage
To run the script run `python3 TestDataGenerator.py` with the following required argument:
```
--splitter_settings     Path to the splitter settings JSON file.
--points                Path to the input spacepoints CSV file.
--points_output         Path to the output spacepoints with region IDs CSV file.
```

Example:
```bash
python3 TestDataGenerator.py --splitter_settings=./splitter_settings.json --points=./event0000-spacepoint.csv --points_output=./event0000-spacepoint-with-region-ids.csv
```

## Input files
### Splitter settings
Defines detector properties, geometry of wedges and pole regions.
```json
{
    "detector_properties": {...},
    "wedges": [...],
    "pole_regions": [...]
}
```

#### Detector properties
* `max_abs_xy` - max absolute value of points' x and y
* `max_abs_z` - max absolute value of points' z
```json
"detector_properties": {
    "max_abs_xy": 1100,
    "max_abs_z": 3100
}
```

#### Wedges
A list of wedges. Each one consists of following attributes:
* `id` - region id
* `z_angle_min` - lower boundary of angle span in plane perpendicular to z axis [rad]
* `z_angle_max` - higher boundary of angle span in plane perpendicular to x axis [rad]
* `x_angle_min` - lower boundary of angle span in plane perpendicular to x axis [rad]
* `x_angle_max` - higher boundary of angle span in plane perpendicular to x axis [rad]
* `interaction_region_width` - distance between the ends of interaction regions
```json
"wedges": [
    {
        "id": 21,
        "z_angle_min": 1.1,
        "z_angle_max": 2.2,
        "x_angle_min": 1.3,
        "x_angle_max": 2.4,
        "interaction_region_width": 420
    },
    {
        "id": 37,
        "z_angle_min": 2.5,
        "z_angle_max": 3.6,
        "x_angle_min": 2.7,
        "x_angle_max": 3.8,
        "interaction_region_width": 420
    }
]
```

#### Pole regions
A list of pole regions. Each one consists of following attributes:
* `id` - region id
* `x_angle` - maximal (if $< \frac{\pi}{2}$) or minimal (if $> \frac{\pi}{2}$) angle between z axis, farther end of the interacion region, and the point
* `interaction_region_width` - distance between the ends of interaction regions
```json
"pole_regions": [
    {
        "id": 21,
        "x_angle": 0.5,
        "interaction_region_width": 420
    },
    {
        "id": 37,
        "x_angle": 2.5,
        "interaction_region_width": 420
    }
]
```

#### Full splitter settings json example
```json
{
    "detector_properties": {
        "max_abs_xy": 1100,
        "max_abs_z": 3100
    },
    "wedges": [
        {
            "id": 1,
            "z_angle_min": 1,
            "z_angle_max": 2,
            "x_angle_min": 1,
            "x_angle_max": 2,
            "interaction_region_width": 420
        },
        {
            "id": 2,
            "z_angle_min": 2,
            "z_angle_max": 3,
            "x_angle_min": 2,
            "x_angle_max": 3,
            "interaction_region_width": 420
        }
    ],
    "pole_regions": [
        {
            "id": 3,
            "x_angle": 0.5,
            "interaction_region_width": 420
        },
        {
            "id": 4,
            "x_angle": 2.5,
            "interaction_region_width": 420
        }
    ]
}
```

### Points CSV
A file containg a list of points in CSV format with column names in the first row and ',' as separator. The file has to contain columns `measurement_id`, `geometry_id`, `x`, `y`, `z`, and optionally `var_r` and `var_z`. Columns have to be in that order. Only `x`, `y`, and `z` are actually used.

#### Example
```csv
measurement_id,geometry_id,x,y,z,var_r,var_z
0,1152921779484760320,156.76506,-5.30625391,-1523.19995,0.000899999985,8.43614858e-37
1,1152921779484760576,153.652435,25.7113953,-1524.40002,0.000899999985,8.43614858e-37
2,1152921779484760576,154.974457,28.9229641,-1524.40002,0.000899999985,8.43614858e-37
3,1152921779484760832,-15.4573355,157.548386,-1523.19995,0.000899999985,8.43614858e-37
4,1152921779484760832,-28.235096,155.294205,-1523.19995,0.000899999985,8.43614858e-37
5,1152921779484760832,-25.5602283,166.976089,-1523.19995,0.000899999985,8.43614858e-37
```

## Output file
A file containg a list of points with associated region ids in CSV format with column names in the first row and ', ' as separator. The file contains columns `x`, `y`, `z`, and `region_ids`. Column `region_ids` contains a list of associated region ids sparated by ', '.

#### Example
```csv
x, y, z, region_ids
-132.252518, 102.947372, -1123.19995, 2, 4
-97.3008423, 77.7407761, -1123.19995, 4
-109.816261, 93.6373749, -1123.19995, 2, 4
-130.774078, 68.4135666, -1124.40002, 2, 4
-120.877335, 67.821312, -1124.40002, 4
```