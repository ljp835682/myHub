using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TileSpawner : MonoBehaviour {

    public GameObject tilePrefeb;           //복사할 게임 오브젝트

	// Use this for initialization
	void Start ()
    {
        MazeCreater mazecreator = GameObject.Find("MazeCreater").GetComponent<MazeCreater>();

        bool colorFlag = false;
        for (float i = 1; i <= 11; i++)
        {
            for (float j = 1; j <= 11; j++)
            {
                GameObject tile = Instantiate(tilePrefeb, new Vector3(j, 0, i), Quaternion.identity);           //지정된위치에 복사 게임 오브젝트 출현              

                if (colorFlag)
                {
                    colorFlag = false;
                    tile.GetComponent<MeshRenderer>().material.color = Color.black;
                }

                else
                {
                    colorFlag = true;
                    tile.GetComponent<MeshRenderer>().material.color = Color.white;
                }

                if (j == mazecreator.GetGoalX() && i == mazecreator.GetGaolY())
                {
                    tile.GetComponent<MeshRenderer>().material.mainTexture = Resources.Load("Texture/Stair") as Texture;      //골지점만 특수
                }
            }
        }
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}
