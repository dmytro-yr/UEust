import numpy as np
from scipy.spatial.transform import Rotation as R

def parse_input_data(input_data):

    """(X=-0.008062,Y=-0.051061,Z=10.182810) (Pitch=0.166441,Yaw=-0.032605,Roll=-0.045904)"""

    # Split the input data into position and rotation parts
    pos_part, rot_part = input_data.split(')(')
    
    # Remove the parentheses and split by commas
    pos_values = pos_part.strip('()').split(',')
    rot_values = rot_part.strip('()').split(',')
    
    # Extract position values
    pos_dict = {}
    for val in pos_values:
        key, value = val.split('=')
        pos_dict[key.strip()] = float(value.strip())
    
    # Extract rotation values
    rot_dict = {}
    for val in rot_values:
        key, value = val.split('=')
        rot_dict[key.strip()] = float(value.strip())
    
    return pos_dict, rot_dict

def calculate_imported_pivot_offset(orig_pos, orig_rot, new_pos, new_rot):

    r_orig = R.from_euler('zyx',  [orig_rot['Yaw'], orig_rot['Pitch'], orig_rot['Roll']], degrees=True)

    # calculate import rotation
    r_import = r_orig.inv()


    #convert to UE

    p_orig = np.array([orig_pos['X'], orig_pos['Y'], orig_pos['Z']])
    p_target = np.array([new_pos['X'], new_pos['Y'], new_pos['Z']])

    # rotate the original position by the import rotation
    p_rotated = r_import.apply(p_target - p_orig)
    import_rot_euler = r_import.as_euler('zyx', degrees=True)

    import_rot_ue = {
        'Roll': import_rot_euler[2],
        'Pitch': import_rot_euler[0],
        'Yaw': import_rot_euler[1]
    }

    import_pos_ue = {
        'X': p_rotated[0],
        'Y': p_rotated[1],
        'Z': p_rotated[2]
    }
    
    return import_pos_ue, import_rot_ue

target_pos = {'X':0.0, 'Y':0.0, 'Z':10.0}
target_rot = {'Pitch':0.0, 'Yaw':0.0, 'Roll':0.0}

if __name__ == "__main__":
    input_data = input("Enter the original position and rotation (e.g. (X=-0.008062,Y=-0.051061,Z=10.182810) (Pitch=0.166441,Yaw=-0.032605,Roll=-0.045904)): ")
    orig_pos, orig_rot = parse_input_data(input_data)
    import_pos, import_rot = calculate_imported_pivot_offset(orig_pos, orig_rot, target_pos, target_rot)

    print(f"Import Position Offset:")
    print(f"  X={import_pos['X']:.6f}")
    print(f"  Y={import_pos['Y']:.6f}") 
    print(f"  Z={import_pos['Z']:.6f}")
    print(f"Import Rotation Offset:")
    print(f"  Roll={import_rot['Roll']:.6f}")
    print(f"  Yaw={import_rot['Yaw']:.6f}")
    print(f"  Pitch={import_rot['Pitch']:.6f}")