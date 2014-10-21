-- a table for the mappable pieces.
--
-- this table was generated by comparing assembled arms
-- using MUMmer3.13 to find a set of oriented matches
-- 3L has one rearrangment; other arms have matches in order

create table r4_to_r5_mapping ( scaffold varchar(10) not null,
                                r4_start integer not null,
                                r5_start integer not null,
                                length integer not null);
grant select on r4_to_r5_mapping to public;

copy r4_to_r5_mapping from stdin;
2R	337	380451	619
2R	3289	383407	1181
2R	5024	385142	196672
2R	201697	581815	77429
2R	279127	659244	1563516
2R	1842644	2222761	185
2R	1842830	2222947	2157
2R	1844995	2225104	14706
2R	1859788	2239897	350
2R	1860191	2240300	75
2R	1861278	2240417	52
2R	1861331	2240470	73
2R	1861405	2240544	419
2R	1861825	2240964	809
2R	1862634	2244528	13458
2R	1876093	2257987	492
2R	1876690	2258584	10466
2R	1887157	2269051	13654
2R	1910815	2284337	8277
2R	1919092	2294784	4216705
2R	6135798	6511490	7100
2R	6149065	6520663	3556878
2R	9705944	10077542	3069
2R	9709013	10081655	6583503
2R	16292616	16672539	4474170
2L	1	1	14739157
2L	14739158	14743229	5447881
2L	20187039	20198026	1240
2L	20188279	20200680	10590
2L	20199259	20211666	1273873
2L	21473232	21485639	934603
3R	1	1	27905053
3L	2307	18776	59170
3L	61478	77946	4252580
3L	4314058	4330527	30908
3L	4345066	4362467	60436
3L	4405503	4422904	15862
3L	4421366	4438766	12248
3L	4433614	4452767	655000
3L	5088714	5107867	4108933
3L	9197648	9216801	4578994
3L	13776644	13795795	11014
3L	13787658	13815499	5903430
3L	19691088	19750345	99462
3L	19790550	19849809	3501711
3L	23313618	23359978	2691
3L	23316309	23362670	415
3L	23316725	23363085	8411
3L	23325138	23371506	411
3L	23325550	23371918	6874
3L	23292934	23379661	6232
3L	23332424	23385892	30556
3L	23362981	23416449	261455
3L	23624436	23677905	51314
3L	23675751	23729220	340
3L	23676092	23729561	46957
3L	23723056	23776524	45006
3L	23768063	23821531	3803
4	1	1	118822
4	118823	119199	1102090
4	1221013	1221389	60628
X	1	8102	5833
X	5840	13941	118
X	5959	14060	199
X	6159	14260	118
X	6278	14379	5050
X	11328	19431	49012
X	60340	69224	6972
X	67313	76197	37
X	67359	76243	40
X	67400	76284	30
X	67431	76315	7738
X	75177	84060	15780
X	90969	99840	150
X	91119	102103	858
X	91977	110790	235
X	92312	129523	305190
X	397502	434714	977429
X	1379722	1417597	91
X	1379813	1417690	2045318
X	3425132	3463009	28630
X	3454174	3491639	3840
X	3458294	3507490	1773
X	3463123	3511432	58544
X	3521668	3569976	16190
X	3537859	3586166	13726
X	3551762	3600061	3793
X	3555559	3603857	12443
X	3568003	3616300	2633
X	3570637	3618933	2102
X	3572740	3621035	383
X	3575512	3624311	313
X	3575825	3624625	6
X	3575845	3624645	12657
X	3588514	3637312	44275
X	3632791	3681587	1944607
X	5577398	5626195	3196315
X	8773813	8822623	16360
X	8790173	8838984	49490
X	8839663	8888475	840556
X	9680275	9729087	629681
X	10309956	10358771	1121754
X	11431810	11486133	1754
X	11433564	11489980	2848
X	11436972	11493253	33777
X	11470776	11527054	5156
X	11476032	11532294	7490
X	11483522	11540401	644
X	11484474	11541045	466
X	11485249	11541511	90639
X	11575988	11636085	42338
X	11618327	11678424	53799
X	11672127	11732223	2443373
X	14115501	14175597	5882986
X	19998487	20058838	1559426
X	21558013	21684550	2794
X	21560907	21759344	663484
\.

-- Create a function that returns a release 5 coordinate.
-- this function is '1' based.

-- this returns a null for any unmappable base. 
--
create function "r4_map" (character varying,integer) returns integer as '
select r5_start+($2-r4_start) from r4_to_r5_mapping where
scaffold = $1 and
r4_start <= $2 and
r4_start+length > $2' language 'sql';

-- the complemetary r5_map maps from release 5 to release 4
create function "r5_map" (character varying,integer) returns integer as '
select r4_start+($2-r5_start) from r4_to_r5_mapping where
scaffold = $1 and
r5_start <= $2 and
r5_start+length > $2' language 'sql';
